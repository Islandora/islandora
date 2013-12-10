<?php

/**
 * @file
 * Assertions for various datastream types.
 *
 * For a datastream validator to work correctly with IslandoraWebTestCase::
 * validateDatastreams(), it needs to return an array of results, each entry of
 * which contains two values: first, TRUE or FALSE, depending on whether or not
 * that particular result passed or failed, and second, a string containing a
 * message to accompany the result.
 *
 * It also should contain three parameters, all of which may use any label, but
 * must be organized in the following order:
 * $object - an object that the datastream can be loaded from.
 * $datastream - a DSID to pull from $object.
 * $optional_params - a parameter for any data the function requires.
 *
 * When IslandoraWebTestCase::validateDatastreams() is called, it is passed an
 * array of datastreams, each of which is itself an array containing the DSID of
 * the datastream, the middle of the function name (image, pdf, tiff, etc.), and
 * (optional) data to be passed to that third parameter.
 */

/**
 * A function to pass assertions to and receive results from.
 *
 * @param bool $assertion
 *   The if/then statement to validate against.
 * @param array $results
 *   An array of results to append the generated result to.
 * @param string $pass
 *   A message to return if the assertion turns up true.
 * @param string $fail
 *   An optional message to return if the assertion turns up false.
 *   If left empty, the $pass message will be returned.
 *
 * @return array
 *   A result that can be made useful in the validation functions below.
 */
function islandora_assert_valid($assertion, $results, $pass, $fail = NULL) {
  if ($assertion) {
    $result = array(TRUE, $pass);
  }
  else {
    if (isset($fail)) {
      $result = array(FALSE, $fail);
    }
    else {
      $result = array(FALSE, $pass);
    }
  }
  array_push($results, $result);
  return $results;
}

/**
 * Converts a hexidecimal string to an integer.
 *
 * This is useful for running checks on values that appear in the binary
 * of a datastream. Returns FALSE if the hex value contains non-hex characters
 * or if the string would not return a 16- or 32-bit formatted big-endian
 * signed integer.
 *
 * @param string $hex
 *   The hexidecimal string.
 *
 * @return bool|int
 *   FALSE on failure, or the integer on success.
 */
function islandora_hex2int($hex) {

  // A couple of quick string checks.
  if (!ctype_xdigit($hex)) {
    drupal_set_message(t('String passed to islandora_hex2int() contains non-hexidecimal characters.'), 'error');
    return FALSE;
  }
  if (!strlen($hex) === 4 || !strlen($hex) === 8) {
    drupal_set_message(t('String passed to islandora_hex2int() cannot create a 16- or 32-bit little-endian signed integer'), 'error');
    return FALSE;
  }

  // The actual conversion.
  try {
    $reverse_hex = implode('', array_reverse(str_split($hex, 2)));
    $int = hexdec($reverse_hex);
    return $int;
  }
  catch (Exception $e) {
    throw new Exception('An error occurred during the conversion of hexidecimal to integer.', 0, $e);
  }
}

/**
 * Asserts that an object's given datastreams are common-type image files.
 *
 * Uses PHPGD to run the assertion check. This means that only certain kinds
 * of image files can be checked. Please check the documentation for the PHPGD
 * imagecreatefromstring() function to determine what filetypes are valid.
 *
 * @param AbstractObject $object
 *   The PID of the object.
 * @param string $datastream
 *   A DSID to check that corresponds to a PHPGD-valid image datastream.
 *
 * @return array
 *   A series of TRUE(pass)/FALSE(fail) results paired with result messages.
 */
function islandora_validate_image_datastream($object, $datastream) {
  $datastream_string = $object[$datastream]->content;
  $results = array();
  $pass = "Image datastream {$datastream} is valid.";
  $fail = "Image datastream {$datastream} is either invalid or corrupt.";
  $results = islandora_assert_valid(imagecreatefromstring($datastream_string), $results, $pass, $fail);
  return $results;
}

/**
 * Asserts the validity of any .tif/.tiff datastream.
 *
 * Does not use the islandora_assert_valid() function, as this is not a simple
 * true/false.
 *
 * @param AbstractObject $object
 *   The PID of the object.
 * @param string $datastream
 *   A DSID to check that corresponds to a .tif/.tiff datastream.
 *
 * @return array
 *   A series of TRUE(pass)/FALSE(fail) results paired with result messages.
 */
function islandora_validate_tiff_datastream($object, $datastream) {
  $datastream_string = $object[$datastream]->content;
  $datastream_header_hex = substr(bin2hex($datastream_string), 0, 8);
  $results = array();
  if ($datastream_header_hex == "49492a00") {
    // In this case, the ingested TIFF is designated as using the "Intel
    // byte-order" (e.g. little-endian) by starting with the characters "II"
    // (repeated so that byte order does not yet need to be significant).
    // The number that follows is '42' in little-endian hex, a number of
    // 'deep philosophical significance' to the TIFF format creators.
    array_push($results, array(TRUE, "{$datastream} datastream asserts that it is a valid Intel-byte-orderded TIF/TIFF file."));
  }
  elseif ($datastream_header_hex == "4d4d002a") {
    // In this case, the ingested TIFF is designated as using the "Motorola
    // byte-order" (e.g. big-endian) by starting with the characters "MM"
    // instead. 42 follows once again, this time in big-endian hex.
    array_push($results, array(TRUE, "{$datastream} datastream asserts that it is a valid Motorola-byte-ordered TIF/TIFF file."));
  }
  else {
    array_push($results, array(FALSE, "{$datastream} datastream does not assert that it is a valid TIF/TIFF file."));
  }
  return $results;
}

/**
 * Asserts the validity of any .jp2 datastream.
 *
 * @param AbstractObject $object
 *   The PID of the object.
 * @param string $datastream
 *   A DSID to check that corresponds to a .jp2 datastream.
 *
 * @return array
 *   A series of TRUE(pass)/FALSE(fail) results paired with result messages.
 */
function islandora_validate_jp2_datastream($object, $datastream) {
  $datastream_hex = bin2hex($object[$datastream]->content);
  $results = array();
  // JP2 files begin with an offset header at the second 32-bit integer,
  // 0x6A502020. This header is in all .jp2s, and we check for it here.
  $pass = "{$datastream} datastream begins correctly with the appropriate .jp2 header.";
  $fail = "{$datastream} datastream does not begin with the appropriate .jp2 header.";
  $results = islandora_assert_valid(substr($datastream_hex, 8, 8) == '6a502020', $results, $pass, $fail);
  // JP2 files have their codestream capped with a marker, 0xFFD9. We're
  // just checking for it here to see if the .jp2 encoder finished okay.
  $pass = "{$datastream} datastream ends correctly with the appropriate .jp2 marker.";
  $fail = "{$datastream} datastream does not end with a .jp2 marker; derivative generation was likely interrupted.";
  $results = islandora_assert_valid(substr($datastream_hex, strlen($datastream_hex) - 4, 4) == 'ffd9', $results, $pass, $fail);
  return $results;
}

/**
 * Asserts the validity of any .pdf datastream.
 *
 * @param AbstractObject $object
 *   The PID of the object.
 * @param string $datastream
 *   A DSID to check that corresponds to a .pdf datastream.
 *
 * @return array
 *   A series of TRUE(pass)/FALSE(fail) results paired with result messages.
 */
function islandora_validate_pdf_datastream($object, $datastream) {
  $pdf = $object[$datastream]->content;
  $pdf_version = substr($pdf, 5, 3);
  $results = array();
  $pass = "{$datastream} datastream asserts that it is a valid PDF file using PDF version {$pdf_version}";
  $fail = "{$datastream} datastream binary header appears to be corrupt and missing a valid PDF signature.";
  $results = islandora_assert_valid(substr($pdf, 0, 5) == '%PDF-', $results, $pass, $fail);

  $pdf_streams = substr_count(bin2hex($pdf), '0a73747265616d0a');
  $pass = "{$datastream} datastream reports the existence of {$pdf_streams} PDF streams. Note that an extremely low number could still indicate corruption.";
  $fail = "{$datastream} datastream contains zero PDF streams, and is likely not a PDF file.";
  $results = islandora_assert_valid($pdf_streams, $results, $pass, $fail);

  $pass = "{$datastream} datastream reports the existence of the closing 'EOF' tag required at the end of PDFs";
  $fail = "{$datastream} datastream does not contain the closing 'EOF' tag. If this is the only PDF validation that failed, it is likely that derivative generation was interrupted.";
  $results = islandora_assert_valid(strpos(bin2hex($pdf), '0a2525454f460a'), $results, $pass, $fail);
  return $results;
}

/**
 * Asserts that a string of text shows up inside a datastream.
 *
 * @param AbstractObject $object
 *   The PID of the object.
 * @param string $datastream
 *   A DSID to check that corresponds to a datastream containing text.
 * @param array $text
 *   An array of strings/the number of times it should appear in the datastream.
 *
 * @return array
 *   A series of TRUE(pass)/FALSE(fail) results paired with result messages.
 */
function islandora_validate_text_datastream($object, $datastream, array $text) {
  $results = array();
  $content = $object[$datastream]->content;
  $string_count = substr_count($content, $text[0]);
  $pass = "{$datastream} datastream contains the word(s) '{$text[0]}' repeated {$string_count} time(s) (expected: {$text[1]}).";
  $fail = "{$datastream} datastream contains the word(s) '{$text[0]}' repeated {$string_count} time(s) (expected: {$text[1]}).";
  $results = islandora_assert_valid($string_count == $text[1], $results, $pass, $fail);
  return $results;
}

/**
 * Asserts the validity of any .wav datastraeam.
 *
 * WAV files contain a rigidly detailed header that contains all sorts of fun
 * information we can use to validate things against other things. So, we check
 * rigorously that the header contains properly constructed data by looking to
 * see if certain values are at their expected byte offset. We also compare
 * declared chunk sizes against actual sizes. If any of these are off, WAV
 * players will fail to function.
 *
 * @param AbstractObject $object
 *   The PID of the object.
 * @param string $datastream
 *   A DSID to check that corresponds to a datastream generated via OCR or HOCR.
 *
 * @return array
 *   A series of TRUE(pass)/FALSE(fail) results paired with result messages.
 */
function islandora_validate_wav_datastream($object, $datastream) {
  $results = array();
  $wav = bin2hex($object['OBJ']->content);
  $wav_subchunk2size = islandora_hex2int(substr($wav, 80, 8));
  $wav_samplerate = islandora_hex2int(substr($wav, 48, 8));
  $wav_numchannels = islandora_hex2int(substr($wav, 44, 4));
  $wav_bytespersample = islandora_hex2int(substr($wav, 68, 4)) / 8;
  $wav_numsamples = strlen(substr($wav, 88)) / $wav_numchannels / $wav_bytespersample / 2;
  $magic_number = str_split(substr($wav, 0, 24), 8);

  $pass = "Header of the {$datastream} datastream contains correct file signature";
  $fail = "Header of the {$datastream} datastream contains corrupt file signature";
  $results = islandora_assert_valid($magic_number[0] = '52494646' && $magic_number[2] = '57415645', $results, $pass, $fail);

  $pass = "{$datastream} datastream chunksize in WAV header is correct";
  $fail = "{$datastream} datastream chunksize in WAV header does not match actual chunksize.";
  $results = islandora_assert_valid(islandora_hex2int(substr($wav, 8, 8)) === 36 + $wav_subchunk2size, $results, $pass, $fail);

  $pass = "{$datastream} datastream contains a 'fmt' subchunk.";
  $fail = "{$datastream} datastream is missing the required 'fmt' subchunk.";
  $results = islandora_assert_valid(substr($wav, 24, 8) === '666d7420', $results, $pass, $fail);

  $pass = "{$datastream} datastream byterate in the WAV header is correct.";
  $fail = "{$datastream} datastream byterate in the WAV header does not match actual calculated byterate.";
  $results = islandora_assert_valid(islandora_hex2int(substr($wav, 56, 8)) === $wav_samplerate * $wav_numchannels * $wav_bytespersample, $results, $pass, $fail);

  $pass = "{$datastream} datastream block alignment is set correctly.";
  $fail = "{$datastream} datastream block alignment is off.";
  $results = islandora_assert_valid(islandora_hex2int(substr($wav, 64, 4)) === $wav_numchannels * $wav_bytespersample, $results, $pass, $fail);

  $pass = "{$datastream} datastream contains 'data' subchunk.";
  $fail = "{$datastream} datastream is missing the 'data' subchunk.";
  $results = islandora_assert_valid(substr($wav, 72, 8) === '64617461', $results, $pass, $fail);

  $pass = "{$datastream} datastream 'data' chunk is the correct size.";
  $fail = "{$datastream} datastream 'data' chunk is sized incorrectly.";
  $results = islandora_assert_valid($wav_subchunk2size === $wav_numsamples * $wav_numchannels * $wav_bytespersample, $results, $pass, $fail);

  return $results;
}

/**
 * Asserts the validity of any .mp3 datastream.
 *
 * Our default setup tries to create an MP3 using VBR, but we do some extra
 * checks in case someone turns that off. If the header contains the characters
 * 'Xing', it is flagged as VBR, and we can do an in-depth check on each of the
 * VBR settings. Otherwise, we look for the basic MP3 signature 'fffa' or 'fffb'
 * at the start of the binary.
 *
 * @param AbstractObject $object
 *   The PID of the object.
 * @param string $datastream
 *   A DSID of a datastream corresponding to an mp3 file.
 *
 * @return array
 *   A series of TRUE(pass)/FALSE(fail) results paired with result messages.
 */
function islandora_validate_mp3_datastream($object, $datastream) {
  $results = array();
  $mp3 = bin2hex($object[$datastream]->content);
  $mp3_size = strlen($mp3) / 2;

  // Looks to see if VBR was set properly by LAME. If so, MATH TIME!
  if (strpos($mp3, '58696e67')) {
    $mp3_vbrheader = substr($mp3, strpos($mp3, '58696e67'), 240);

    // Check the field flags. VBR-formatted MP3 files contain a 32-bit
    // integer (stored as $mp3_flag_value) that is a combination of four
    // bits, each one indicating the on-off status of a VBR setting, via
    // logical OR. Rather than disassembling this value into individual
    // bits, we use the algorithm "if (binary_total+bit_value*2)/bit_value*2
    // is greater than or equal to bit_value, that bit is turned on" to find
    // the status of each bit, so we know whether to offset the rest.
    $mp3_field_offset = array(0, 0, 0);
    $mp3_flag_value = hexdec(substr($mp3_vbrheader, 8, 8));

    // We can't use the first flag, but we still need to offset the rest.
    if (($mp3_flag_value + 1) % 2 == 0) {
      $mp3_field_offset[0] += 8;
      $mp3_field_offset[1] += 8;
      $mp3_field_offset[2] += 8;
    }

    // The second flag leads us to filesize data, which we can verify.
    if (($mp3_flag_value + 4) % 4 > 1) {
      $mp3_field_bytes = hexdec(substr($mp3_vbrheader, $mp3_field_offset[0] + 16, 8));
      $pass = "{$datastream} datastream reported filesize of {$mp3_size} bytes matches size field value of {$mp3_field_bytes}";
      $fail = "{$datastream} datastream reported filesize of {$mp3_size} bytes does not match size field value of {$mp3_field_bytes}";
      $results = islandora_assert_valid($mp3_size == $mp3_field_bytes, $results, $pass, $fail);
      $mp3_field_offset[1] += 8;
      $mp3_field_offset[2] += 8;
    }

    // We can't use the third flag for anything either.
    if (($mp3_flag_value + 8) % 8 > 3) {
      $mp3_field_offset[2] += 200;
    }

    // The fourth flag leads us to VBR quality data, which we can validate.
    if ($mp3_flag_value > 7) {
      $mp3_field_quality = hexdec(substr($mp3_vbrheader, $mp3_field_offset[2] + 16, 8));
      $pass = "{$datastream} datastream reports valid VBR quality of {$mp3_field_quality} (expected: between 0-100)";
      $fail = "{$datastream} datastream reports invalid VBR quality of {$mp3_field_quality} (expected: between 0-100)";
      $results = islandora_assert_valid($mp3_field_quality <= 100 && $mp3_field_quality >= 0, $results, $pass, $fail);
    }
  }

  // Otherwise, just forget everything and check the file signature.
  elseif (strpos($mp3, '58696e67') == FALSE && substr($mp3, 0, 4) == 'fffa') {
    $results = array(array(TRUE, "{$datastream} datastream is encoded as a valid MPEG-1 Layer 3 file with CRC protection"));
  }
  elseif (strpos($mp3, '58696e67') == FALSE && substr($mp3, 0, 4) == 'fffb') {
    $results = array(array(TRUE, "{$datastream} datastream is encoded as a valid unprotected MPEG-1 Layer 3 file"));
  }
  else {
    $results = array(array(FALSE, "{$datastream} datastream is corrupt and does not identify as a valid MP3."));
  }
  return $results;
}

/**
 * Attempts to validate an .mp4 datastream.
 *
 * MP4 files are a subset of the ISO file format specification, and as such need
 * to contain a 64-bit declaration of type within the first eight eight bytes of
 * the file. This declaration is comprised of the characters 'ftyp', followed by
 * a four-character filetype code. Below, we look for 'ftyp', and then pass the
 * filetype code to the test message.
 *
 * @param AbstractObject $object
 *   The PID of the object.
 * @param string $datastream
 *   A DSID of a datastream corresponding to an mp4 file.
 *
 * @return array
 *   A series of TRUE(pass)/FALSE(fail) results paired with result messages.
 */
function islandora_validate_mp4_datastream($object, $datastream) {
  $results = array();
  $mp4 = $object[$datastream]->content;
  if (strpos($mp4, 'ftyp')) {
    $mp4_ftyp = substr(strpos($mp4, 'ftyp'), 4, 4);
  }
  $pass = "{$datastream} datastream asserts that it is a valid ISO-formatted video file using ftyp {$mp4_ftyp}";
  $fail = "{$datastream} datastream is not a valid ISO-formatted video";
  $results = islandora_assert_valid(strpos($mp4, 'ftyp'), $results, $pass, $fail);
  return $results;
}

/**
 * Attempts to validate an .ogg/ogv datastream using Vorbis and Theora encoding.
 *
 * OGG files are made up of several 'pages' of OGG data, each prefaced with an
 * OGG marker - the letters 'OggS'. The file header also contains information on
 * what encoders were used to create the file. Here, we're looking for at least
 * one OGG page, and confirming that the file asserts the Theora and Vorbis
 * codecs were used to create the file.
 *
 * @param AbstractObject $object
 *   The PID of the object.
 * @param string $datastream
 *   A DSID of a datastream corresponding to an ogg file.
 *
 * @return array
 *   A series of TRUE(pass)/FALSE(fail) results paired with result messages.
 */
function islandora_validate_ogg_datastream($object, $datastream) {
  $results = array();
  $ogg = $object[$datastream]->content;
  $ogg_pages = substr_count($ogg, 'OggS');

  $pass = "{$datastream} datastream asserts that it contains {$ogg_pages} Ogg pages (even a very small file should contain several).";
  $fail = "{$datastream} datastream contains no Ogg pages.";
  $results = islandora_assert_valid(substr_count($ogg, 'OggS'), $results, $pass, $fail);

  $pass = "{$datastream} datastream asserts that it contains Theora-encoded video data.";
  $fail = "{$datastream} datastream contains no marker indicating the presence of Theora-encoded video data.";
  $results = islandora_assert_valid(substr_count($ogg, 'theora'), $results, $pass, $fail);

  $pass = "{$datastream} datastream asserts that it contains Vorbis-encoded audio data";
  $fail = "{$datastream} datastream contains no marker indicating the presence of Vorbis-encoded audio data.";
  $results = islandora_assert_valid(substr_count($ogg, 'vorbis'), $results, $pass, $fail);

  return $results;
}

/**
 * Attempts to validate an .mkv datastream.
 *
 * There's not much we can do to check an MKV file, since the format is really,
 * really loose. We do know a couple of things though - first, since MKV is an
 * EBML format, the first four characters will always be the same. Since they're
 * non-standard characters, we're looking at their hex values instead. And
 * second, we know that the file will contain the declaration 'matroska' soon
 * after. We could look for this in the binary, but we already have the hex-
 * translated version, so we just look for 'matroska' in hex.
 *
 * @param AbstractObject $object
 *   The PID of the object.
 * @param string $datastream
 *   A DSID of a datastream corresponding to an MKV file.
 *
 * @return array
 *   A series of TRUE(pass)/FALSE(fail) results paired with result messages.
 */
function islandora_validate_mkv_datastream($object, $datastream) {
  $results = array();
  $mkv = bin2hex($object[$datastream]->content);

  $pass = "{$datastream} datastream asserts that it is an EBML-formatted file";
  $fail = "{$datastream} datastream is not an EBML-formatted file.";
  $results = islandora_assert_valid(substr($mkv, 0, 8) == '1a45dfa3', $results, $pass, $fail);

  $pass = "{$datastream} datastream asserts that its EBML DocType is Matroska";
  $fail = "{$datastream} datastream does not contain a Matroska EBML DocType marker.";
  $results = islandora_assert_valid(substr_count($mkv, '6d6174726f736b61') == 1, $results, $pass, $fail);

  return $results;
}
