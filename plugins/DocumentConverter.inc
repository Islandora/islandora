<?php


/**
 * @file
 * Document Converter Class
 */

/**
 * This class implements document (doc, odt, pdf, etc.) conversion for a generic
 * multi-format document collection.
 */
class DocumentConverter {

  private $converter_service_url = "http://localhost:8080/converter/service";

  /**
   * Constructor
   * @param type $converter_url 
   */
  public function __construct($converter_url = NULL) {
    if (!empty($converter_url))
      $this->converter_service_url = $converter_url;
  }

  /**
   * Convert ???
   * @param type $parameterArray
   * @param type $dsid
   * @param type $file
   * @param type $output_ext
   * @return string 
   */
  function convert($parameterArray = NULL, $dsid, $file, $output_ext) {
    module_load_include('inc', 'fedora_repository', 'MimeClass');

    #debug:
    #drupal_set_message("Sending $file to ". $this->converter_service_url ." for convertsion to $output_ext", 'status');

    if (!empty($parameterArray['converter_url'])) {
      $this->converter_service_url = $parameterArray['converter_url'];
    }

    $helper = new MimeClass();
    $inputType = $helper->get_mimetype($file);
    $outputType = $helper->get_mimetype($output_ext);
    $inputData = file_get_contents($file);

    $outputFile = $file . "_" . $dsid . "." . $output_ext;

    #debug:
    #drupal_set_message("inputType: $inputType", 'status');
    #drupal_set_message("outputType: $outputType", 'status');
    #drupal_set_message("outputFile: $outputFile", 'status');

    $ch = curl_init($this->converter_service_url);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array("Content-Type: $inputType", "Accept: $outputType"));
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_TIMEOUT, 120); // times out after 2 minutes 
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); // return into a variable 
    curl_setopt($ch, CURLOPT_POSTFIELDS, $inputData); // add POST fields 
    #curl_setopt($ch, CURLOPT_HEADER, 1);
    #curl_setopt($ch, CURLOPT_VERBOSE, 1);

    $data = curl_exec($ch); // run the whole process 

    $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);

    if (200 == $code) {

      $returnValue = file_put_contents($outputFile, $data);
      if ($returnValue > 0) {
        drupal_set_message(t("Conversion successful."), 'status');
        $_SESSION['fedora_ingest_files']["$dsid"] = $outputFile;
        return $outputFile;
      }
      else {
        return $returnValue; // a.k.a. FALSE.
      }
    }
    else {
      drupal_set_message(t("Conversion Failed. Webservice returned $code."), 'status');
      return FALSE;
    }
  }

}

/*
  $documentConverter = new DocumentConverter();
  $inputFile = "document.docx";
  $outputType = "txt";
  $documentConverter->convert( NULL, 'TXT', $inputFile, $outputType);
  /* */
