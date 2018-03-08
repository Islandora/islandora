<?php

/**
 * @file
 * Classes and functions for datastream validation.
 */

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
 * @throws Exception
 *   If something horrible happens during the actual conversion.
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
  if (!(strlen($hex) == 4 || strlen($hex) == 8)) {
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
 * Abstraction for datastream validators.
 *
 * Classes extended from DatastreamValidator don't require much to be useful.
 * They accept a Fedora object and a DSID to perform assertions on;
 * all you have to do is place a series of functions inside the extended class
 * using the naming convention assertThing(); each of these functions should
 * ideally assert one thing and one thing only (for simplicity's sake), and
 * should generate either a pass or a fail message by calling addPass() or
 * addFail(). That's it, really; they don't have to return any values, as
 * addPass() and addFail() just add messages to the overall pass/fail array.
 *
 * As long as you use those rules and naming conventions, all the magic is done
 * when you instantiate the new datastream validator object.
 *
 * The IslandoraWebTestCase::assertDatastreams() function accepts paired DSIDs
 * and datastream validator names in order to do the rest of the work. It grabs
 * all the test results using getPasses() and getFails() and transforms those
 * into something that DrupalWebTestCase can use.
 */
// @codingStandardsIgnoreLine
abstract class DatastreamValidator extends IslandoraTestUtilityClass {

  /**
   * The Fedora object containing the datastream to test.
   *
   * @var IslandoraFedoraObject|FedoraObject
   */
  public $object;

  /**
   * The DSID of the string to test.
   *
   * @var string
   */
  public $datastream;

  /**
   * The content of the datastream.
   *
   * @var string[]
   */
  public $datastreamContent;

  /**
   * An array of additional required parameters.
   *
   * @var array
   */
  public $params = array();

  /**
   * Constructs a DatastreamValidator.
   *
   * @param IslandoraFedoraObject|FedoraObject $object
   *   The object to grab the datastream from.
   * @param string $datastream
   *   The DSID of the datastream itself.
   * @param array $params
   *   An extra array of parameters the validator might need.
   */
  public function __construct($object, $datastream, array $params = array()) {
    $this->object = $object;
    $this->datastream = $datastream;
    $this->params = $params;
    if ($object[$datastream]) {
      $this->datastreamContent = $object[$datastream]->content;
    }
    else {
      drupal_set_message(t("Error grabbing content from datastream %datastream in object %id", array(
        '%datastream' => $datastream,
        '%id' => $object->id,
      )), 'error');
    }
  }

  /**
   * Helper function to run all the validators in a class.
   *
   * On DatastreamValidator::__construct(), this looks for any functions
   * within the class beginning in "assert" and runs them. In all current cases
   * (and realistically in all future cases), this adds one or more passes or
   * fails to $this->passes and/or $this->fails.
   */
  public function runValidators() {
    if ($this->object[$this->datastream]) {
      $methods = get_class_methods($this);
      foreach ($methods as $method) {
        if (substr($method, 0, 6) === 'assert') {
          $this->$method();
        }
      }
    }
    else {
      $this->addResult(FALSE, "Unable to load the requested datastream {$this->datastream} from object {$this->object->id}.");
    }
  }

  /**
   * Returns an array of IslandoraTestUtilityResults.
   *
   * @return IslandoraTestUtilityResult[]
   *   The results.
   */
  public function getResults() {
    if (empty($this->results)) {
      $this->runValidators();
    }
    return $this->results;
  }

  /**
   * Adds a result to $this->results.
   *
   * @param bool $type
   *   The type of result (TRUE for pass, FALSE for fail).
   * @param string $message
   *   The message to put in the result.
   */
  public function addResult($type, $message) {
    $result = new IslandoraTestUtilityResult($type, $message, $this->getAssertionCall());
    $this->results[] = $result;
  }

  /**
   * Cycles through backtrace until the first non-assertion method is found.
   *
   * This is a manipulated version of DrupalWebTestCase::getAssertionCall().
   * We use it here so that we can pass back assertion calls from
   * DatastreamValidator assertions instead of less useful TestCase functions.
   *
   * @return array
   *   Array representing the true caller.
   */
  public function getAssertionCall() {
    $backtrace = debug_backtrace();

    // While the current caller's function starts with 'assert', and another one
    // exists after this function, keep poppin' em off.
    while (substr($backtrace[1]['function'], 0, 6) !== 'assert' && isset($backtrace[2])) {
      array_shift($backtrace);
    }

    return _drupal_get_last_caller($backtrace);
  }

}

/**
 * Asserts that an object's given datastreams are common-type image files.
 *
 * Uses PHPGD to run the assertion check. This means that only certain kinds
 * of image files can be checked. Please check the documentation for the PHPGD
 * imagecreatefromstring() function to determine what filetypes are valid.
 */
// @codingStandardsIgnoreLine
class ImageDatastreamValidator extends DatastreamValidator {

  /**
   * Asserts the validity of an image using PHPGD.
   */
  protected function assertImageGeneration() {
    $assertion = imagecreatefromstring($this->datastreamContent) !== FALSE;
    $pass = "Image datastream {$this->datastream} is valid.";
    $fail = "Image datastream {$this->datastream} is either invalid or corrupt.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

}

/**
 * Asserts the validity of any .tif/.tiff datastream.
 */
// @codingStandardsIgnoreLine
class TIFFDatastreamValidator extends DatastreamValidator {

  /**
   * Asserts that the TIFF contains an appropriate header.
   */
  public function assertTiffHeaderHex() {
    $datastream_header_hex = self::getTIFFHeaderHex();
    if ($datastream_header_hex == "49492a00") {
      // In this case, the ingested TIFF is designated as using the "Intel
      // byte-order" (i.e. little-endian) by starting with the characters "II"
      // (repeated so that byte order does not yet need to be significant).
      // The number that follows is '42' in little-endian hex, a number of
      // 'deep philosophical significance' to the TIFF format creators.'.
      $this->addResult(TRUE, "{$this->datastream} datastream asserts that it is a valid Intel-byte-orderded TIF/TIFF file.");
    }
    elseif ($datastream_header_hex == "4d4d002a") {
      // In this case, the ingested TIFF is designated as using the "Motorola
      // byte-order" (i.e. big-endian) by starting with the characters "MM"
      // instead. 42 follows once again, this time in big-endian hex.
      $this->addResult(TRUE, "{$this->datastream} datastream asserts that it is a valid Motorola-byte-ordered TIF/TIFF file.");
    }
    else {
      $this->addResult(FALSE, "{$this->datastream} datastream does not assert that it is a valid TIF/TIFF file.");
    }
  }

  /**
   * Grabs the first 8 characters from the TIFF datastream's hex.
   *
   * @return string
   *   The ... thing I just wrote up there.
   */
  protected function getTiffHeaderHex() {
    return substr(bin2hex($this->datastreamContent), 0, 8);
  }

}

/**
 * Asserts the validity of a JP2 datastream.
 */
// @codingStandardsIgnoreLine
class JP2DatastreamValidator extends DatastreamValidator {

  /**
   * Asserts the hex values at the head of the JP2 file.
   *
   * JP2 files begin with an offset header at the second 32-bit integer,
   * 0x6A502020. This header is in all .jp2s, and we check for it here.
   */
  protected function assertJp2Header() {
    $assertion = substr(bin2hex($this->datastreamContent), 8, 8) == '6a502020';
    $pass = "Datastream {$this->datastream} contains the appropriate JP2 header.";
    $fail = "Datastream {$this->datastream} does not contain the appropriate JP2 header.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Asserts the marker at the end of the JP2 file.
   *
   * JP2 files have their codestream capped with a marker, 0xFFD9. We're just
   * checking for it here to see if the .jp2 encoder finished okay.
   */
  protected function assertJp2Marker() {
    $assertion = substr(bin2hex($this->datastreamContent), strlen(bin2hex($this->datastreamContent)) - 4, 4) == 'ffd9';
    $pass = "Datastream {$this->datastream} contains the appropriate JP2 ending marker.";
    $fail = "Datastream {$this->datastream} does not contain the appropriate JP2 ending marker. If this is the only JP2 validator that failed, it is likely that derivative generation was interrupted.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

}

/**
 * Asserts the validity of a PDF datastream.
 */
// @codingStandardsIgnoreLine
class PDFDatastreamValidator extends DatastreamValidator {

  /**
   * Validates the PDF signature.
   */
  protected function assertPdfSignature() {
    $assertion = substr($this->datastreamContent, 0, 5) == '%PDF-';
    $pdf_version = substr($this->datastreamContent, 5, 3);
    $pass = "{$this->datastream} datastream asserts that it is a valid PDF file using PDF version {$pdf_version}";
    $fail = "{$this->datastream} datastream binary header appears to be corrupt and missing a valid PDF signature.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Counts the number of signatures in this PDF file and asserts there are any.
   */
  protected function assertPdfStreamCount() {
    $pdf_stream_count = substr_count(bin2hex($this->datastreamContent), '0a73747265616d0a');
    $assertion = $pdf_stream_count !== 0;
    $pass = "{$this->datastream} datastream reports the existence of {$pdf_stream_count} PDF streams. Note that an extremely low number could still indicate corruption.";
    $fail = "{$this->datastream} datastream contains zero PDF streams, and is likely not a PDF file.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Validates the PDF closing tag.
   */
  protected function assertPdfClosingTag() {
    $assertion = strpos(bin2hex($this->datastreamContent), '0a2525454f460a') == TRUE;
    $pass = "{$this->datastream} datastream reports the existence of the closing 'EOF' tag required at the end of PDFs";
    $fail = "{$this->datastream} datastream does not contain the closing 'EOF' tag. If this is the only PDF validation that failed, it is likely that derivative generation was interrupted.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

}

/**
 * Validates the number of times a string occurs in a datastream.
 *
 * Requires $this->params to be set to an array containing two keys - the first
 * is the string we're looking to find in the datastream, and the second is an
 * integer representing the number of times it should appear in the datastream.
 */
// @codingStandardsIgnoreLine
class TextDatastreamValidator extends DatastreamValidator {

  /**
   * Constructor override; blow up if we don't have our two values.
   */
  public function __construct($object, $datastream, array $params = array()) {
    if (count($params) < 2) {
      throw new InvalidArgumentException('$params must contain at least two values to instantiate a TextDatastreamValidator.');
    }
    parent::__construct($object, $datastream, $params);
  }

  /**
   * Asserts that the string given appears the correct number of times.
   */
  protected function assertTextStringCount() {
    $string_count = self::getTextStringCount();
    list($string, $expected) = $this->params;
    $assertion = $string_count === $expected;
    $this->addResult($assertion, "{$this->datastream} datastream contains the word(s) '{$string}' repeated {$string_count} time(s) (expected: {$expected}).");
  }

  /**
   * The number of times key [0] in $this->params appears in the datastream.
   *
   * @return int
   *   That count I just mentioned up there.
   */
  protected function getTextStringCount() {
    return substr_count($this->datastreamContent, $this->params[0]);
  }

}

/**
 * Asserts the validity a WAV datastream.
 *
 * WAV files contain a rigidly detailed header that contains all sorts of fun
 * information we can use to validate things against other things. So, we check
 * rigorously that the header contains properly constructed data by looking to
 * see if certain values are at their expected byte offset. We also compare
 * declared chunk sizes against actual sizes. If any of these are off, WAV
 * players will fail to function.
 */
// @codingStandardsIgnoreLine
class WAVDatastreamValidator extends DatastreamValidator {

  /**
   * We need a special constructor here to get the hex datastream content.
   *
   * @param IslandoraFedoraObject|FedoraObject $object
   *   The object to grab the datastream from.
   * @param string $datastream
   *   The DSID of the datastream itself.
   * @param array $params
   *   An extra array of parameters the validator might need.
   */
  public function __construct($object, $datastream, array $params = array()) {
    parent::__construct($object, $datastream, $params);
    $this->datastreamContent = bin2hex($this->datastreamContent);
  }

  /**
   * Asserts that the datastream contains a valid WAV signature.
   */
  protected function assertWavSignature() {
    $signatures = str_split(substr($this->datastreamContent, 0, 24), 8);
    $assertion = $signatures[0] == '52494646' && $signatures[2] == '57415645';
    $pass = "Header of the {$this->datastream} datastream contains a valid file signature.";
    $fail = "Header of the {$this->datastream} datastream contains corrupt file signature.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Asserts that the chunksize in the header is correct.
   */
  protected function assertWavChunkSize() {
    $assertion = islandora_hex2int(substr($this->datastreamContent, 8, 8)) === 36 + self::getDataSubChunkSize();
    $pass = "{$this->datastream} datastream chunksize in WAV header is correct";
    $fail = "{$this->datastream} datastream chunksize in WAV header does not match actual chunksize.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Asserts that the datastream contains a 'fmt' subchunk.
   */
  protected function assertWavFmtSubChunk() {
    $assertion = substr($this->datastreamContent, 24, 8) === '666d7420';
    $pass = "{$this->datastream} datastream contains a 'fmt' subchunk.";
    $fail = "{$this->datastream} datastream is missing the required 'fmt' subchunk.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Asserts that the byterate reported by the WAV header is valid.
   */
  protected function assertWavByteRate() {
    $wav_samplerate = islandora_hex2int(substr($this->datastreamContent, 48, 8));
    $assertion = islandora_hex2int(substr($this->datastreamContent, 56, 8)) === $wav_samplerate * self::getNumChannels() * self::getBytesPerSample();
    $pass = "{$this->datastream} datastream byterate in the WAV header is correct.";
    $fail = "{$this->datastream} datastream byterate in the WAV header does not match actual calculated byterate.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Asserts that the block alignment is correct.
   */
  protected function assertWavBlockAlignment() {
    $assertion = islandora_hex2int(substr($this->datastreamContent, 64, 4)) === self::getNumChannels() * self::getBytesPerSample();
    $pass = "{$this->datastream} datastream block alignment is set correctly.";
    $fail = "{$this->datastream} datastream block alignment is off.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Asserts the existence of a 'data' subchunk.
   *
   * Also asserts that the subchunk size is correct.
   */
  protected function assertWavDataSubChunk() {
    if (substr($this->datastreamContent, 72, 8) !== '64617461') {
      $this->addResult(FALSE, "{$this->datastream} datastream is missing the 'data' subchunk.");
      return;
    }
    else {
      $this->addResult(TRUE, "{$this->datastream} datastream contains 'data' subchunk.");
      $wav_numsamples = strlen(substr($this->datastreamContent, 88)) / self::getNumChannels() / self::getBytesPerSample() / 2;
      $assertion = self::getDataSubChunkSize() === $wav_numsamples * self::getNumChannels() * self::getBytesPerSample();
      $pass = "{$this->datastream} datastream 'data' chunk is the correct size.";
      $fail = "{$this->datastream} datastream 'data' chunk is sized incorrectly.";
      $message = $assertion ? $pass : $fail;
      $this->addResult($assertion, $message);
    }
  }

  /**
   * Gets the number of channels reported by the WAV header.
   *
   * @return int
   *   The number of channels reported by the datastream header.
   */
  protected function getNumChannels() {
    return islandora_hex2int(substr($this->datastreamContent, 44, 4));
  }

  /**
   * Gets the reported number of byte rates per sample.
   *
   * @return int
   *   The number of bytes per sample reported by the datastream header.
   */
  protected function getBytesPerSample() {
    return islandora_hex2int(substr($this->datastreamContent, 68, 4)) / 8;
  }

  /**
   * Gets the size of the 'data' subchunk.
   *
   * @return int
   *   The size of the 'data' subchunk.
   */
  protected function getDataSubChunkSize() {
    return islandora_hex2int(substr($this->datastreamContent, 80, 8));
  }

}

/**
 * Asserts the validity of any .mp3 datastream.
 *
 * Our default setup tries to create an MP3 using VBR, but we do some extra
 * checks in case someone turns that off. If the header contains the characters
 * 'Xing', it is flagged as VBR, and we can do an in-depth check on each of the
 * VBR settings. Otherwise, we look for the basic MP3 signature 'fffa' or 'fffb'
 * at the start of the binary.
 */
// @codingStandardsIgnoreLine
class MP3DatastreamValidator extends DatastreamValidator {

  /**
   * Asserts the validity of the MP3.
   *
   * The MP3 file format is a bit of a mess; the entire makeup of the file
   * depends on whether it uses variable bit rate or static bit rate. So, I'm
   * breaking my own rules here and using a single assert function so that I
   * can handle the weird logic.
   */
  protected function assertValidMp3() {
    $this->datastreamContent = bin2hex($this->datastreamContent);

    // If it's not a VBR MP3, we don't have to check much, so let's get that
    // out of the way first before we go doing a bunch of potentially pointless
    // math. Check to see if the VBR flag (58696e67) isn't there.
    if (strpos($this->datastreamContent, '58696e67') == FALSE && substr($this->datastreamContent, 0, 4) == 'fffa') {
      $this->addResult(TRUE, "{$this->datastream} datastream is encoded as a valid MPEG-1 Layer 3 file with CRC protection");
      return;
    }
    if (strpos($this->datastreamContent, '58696e67') == FALSE && substr($this->datastreamContent, 0, 4) == 'fffb') {
      $this->addResult(TRUE, "{$this->datastream} datastream is encoded as a valid unprotected MPEG-1 Layer 3 file");
      return;
    }

    // And what if the flag IS set?
    if (strpos($this->datastreamContent, '58696e67')) {
      // Check the field flags. VBR-formatted MP3 files contain a 32-bit
      // integer (stored as $mp3_flag_value) that is a combination of four
      // bits, each one indicating the on-off status of a VBR setting, via
      // logical OR. Rather than disassembling this value into individual
      // bits, we use the algorithm "if (binary_total+bit_value*2)/bit_value*2
      // is greater than or equal to bit_value, that bit is turned on" to find
      // the status of each bit, so we know whether to offset the rest.
      $mp3_field_offset = array(0, 0, 0);
      $mp3_vbrheader = substr($this->datastreamContent, strpos($this->datastreamContent, '58696e67'), 240);
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
        $mp3_size = strlen($this->datastreamContent) / 2;
        $assertion = $mp3_size == $mp3_field_bytes;
        $pass = "{$this->datastream} datastream reported filesize of {$mp3_size} bytes matches size field value of {$mp3_field_bytes}";
        $fail = "{$this->datastream} datastream reported filesize of {$mp3_size} bytes does not match size field value of {$mp3_field_bytes}";
        $message = $assertion ? $pass : $fail;
        $this->addResult($assertion, $message);
        $mp3_field_offset[1] += 8;
        $mp3_field_offset[2] += 8;
      }

      // We can't use the third flag for anything, but we still have to offset.
      if (($mp3_flag_value + 8) % 8 > 3) {
        $mp3_field_offset[2] += 200;
      }

      // The fourth flag leads us to VBR quality data, which we can validate.
      if ($mp3_flag_value > 7) {
        $mp3_field_quality = hexdec(substr($mp3_vbrheader, $mp3_field_offset[2] + 16, 8));
        $assertion = $mp3_field_quality <= 100 && $mp3_field_quality >= 0;
        $pass = "{$this->datastream} datastream reports valid VBR quality of {$mp3_field_quality} (expected: between 0-100)";
        $fail = "{$this->datastream} datastream reports invalid VBR quality of {$mp3_field_quality} (expected: between 0-100)";
        $message = $assertion ? $pass : $fail;
        $this->addResult($assertion, $message);
      }
    }

    // If none of that works out, fail.
    else {
      $this->addResult(FALSE, "{$this->datastream} datastream is corrupt and does not identify as a valid MP3.");
    }
  }

}

/**
 * Attempts to validate an .mp4 datastream.
 *
 * MP4 files are a subset of the ISO file format specification, and as such need
 * to contain a 64-bit declaration of type within the first eight eight bytes of
 * the file. This declaration is comprised of the characters 'ftyp', followed by
 * a four-character filetype code. Here, we look for 'ftyp', and then pass the
 * filetype code to the test message.
 */
// @codingStandardsIgnoreLine
class MP4DatastreamValidator extends DatastreamValidator {

  /**
   * Asserts that the datastream is ISO-formatted video.
   */
  protected function assertIsoVideo() {
    $mp4_ftyp = substr(strpos($this->datastreamContent, 'ftyp'), 4, 4);
    $assertion = strpos($this->datastreamContent, 'ftyp') !== 0;
    $pass = "{$this->datastream} datastream asserts that it is a valid ISO-formatted video file using ftyp {$mp4_ftyp}";
    $fail = "{$this->datastream} datastream is not a valid ISO-formatted video";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

}

/**
 * Attempts to validate an .ogg/ogv datastream using Vorbis and Theora encoding.
 *
 * OGG files are made up of several 'pages' of OGG data, each prefaced with an
 * OGG marker - the letters 'OggS'. The file header also contains information on
 * what encoders were used to create the file. Here, we're looking for at least
 * one OGG page, and confirming that the file asserts the Theora and Vorbis
 * codecs were used to create the file.
 */
// @codingStandardsIgnoreLine
class OGGDatastreamValidator extends DatastreamValidator {

  /**
   * Asserts that the datastream contains ogg pages.
   */
  protected function assertOggPages() {
    $ogg_pages = substr_count($this->datastreamContent, 'OggS');
    $assertion = $ogg_pages !== 0;
    $pass = "{$this->datastream} datastream asserts that it contains {$ogg_pages} Ogg pages (even a very small file should contain several).";
    $fail = "{$this->datastream} datastream contains no Ogg pages.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Asserts that the datastream contains Theora-encoded video.
   */
  protected function assertTheoraVideo() {
    $assertion = substr_count($this->datastreamContent, 'theora') !== 0;
    $pass = "{$this->datastream} datastream asserts that it contains Theora-encoded video data.";
    $fail = "{$this->datastream} datastream contains no marker indicating the presence of Theora-encoded video data.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Asserts that the datastream contains Vorbis-encoded audio.
   */
  protected function assertVorbisAudio() {
    $assertion = substr_count($this->datastreamContent, 'vorbis') !== 0;
    $pass = "{$this->datastream} datastream asserts that it contains Vorbis-encoded audio data";
    $fail = "{$this->datastream} datastream contains no marker indicating the presence of Vorbis-encoded audio data.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

}

/**
 * Attempts to validate an .mkv datastream.
 *
 * There's not much we can do to check an MKV file, since the format is really,
 * really loose. We do know a couple of things though - first, since MKV is an
 * EBML format, the first four characters will always be the same. Since they're
 * non-standard characters, we're looking at their hex values instead. And
 * second, we know that the file will contain the declaration 'matroska' soon
 * after.
 */
// @codingStandardsIgnoreLine
class MKVDatastreamValidator extends DatastreamValidator {

  /**
   * Asserts that the datastream is an EBML-format file.
   */
  protected function assertEbmlFormat() {
    $assertion = substr(bin2hex($this->datastreamContent), 0, 8) == '1a45dfa3';
    $pass = "{$this->datastream} datastream asserts that it is an EBML-formatted file";
    $fail = "{$this->datastream} datastream is not an EBML-formatted file.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

  /**
   * Asserts that the datastream contains a matroska marker.
   */
  protected function assertMatroskaMarker() {
    $assertion = substr_count($this->datastreamContent, 'matroska') == 1;
    $pass = "{$this->datastream} datastream asserts that its EBML DocType is Matroska";
    $fail = "{$this->datastream} datastream does not contain a Matroska EBML DocType marker.";
    $message = $assertion ? $pass : $fail;
    $this->addResult($assertion, $message);
  }

}
