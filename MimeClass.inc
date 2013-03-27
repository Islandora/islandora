<?php


/**
 * @file
 *
 * Replacement for the original islandora MimeClass. 
 * Does essentially the same thing, just using the built in drupal 7 
 * mime system. 
 * 
 * A nice added bonus is that this new class will put a note in watchdog when 
 * it encounters a filetype it doesnt know. 
 *
 */

class MimeClass {

  private $mime_types;
  private $ext_types;

  /**
   * Construtor
   */
  public function __construct() {
    //Drupal returns the mimetype list as a double assoc array with two entries: 
	//mimetypes: Integer array of mimetypes
	//extensions: Assoc array of extensions 
	//
	//The extension array can be used as a key to get the value in the mimetypes array 
	//Example  if $extensions['png'] == 5, then $mimetypes[5] = 'image/png'
	//we want to combine this into 1 array for easy access.
	
    $temp = file_default_mimetype_mapping();

	$temp_mime_types = $temp['mimetypes'];
	$temp_mime_ext = $temp['extensions'];
	

	unset($temp); 
	
	//combine the two arrays into one. 
	foreach($temp_mime_ext as $key=>$val)
	{
		$this->mime_types[$key] = $temp_mime_types[$val];
	}
	
	//flip the mime_type array to make the ext array
	$this->ext_types = array_flip($this->mime_types);

  }

  /**
   * function: getType
   * description: An alias to get_mimetype,
   * for backwards-compatibility with our old mimetype class.
   *
   * @param type $filename
   * @return type
  */
    function getType($filename) {
    return $this->get_mimetype($filename);
  }

  /**
   * function: get_mimetype
   * description: returns a mimetype associated with the
   *   file extension of $filename
   *
   * @param type $filename
   * @param type $debug
   * @return string
   *   mimetype associated with the file extension of $filename
   */
  public function get_mimetype($filename, $debug = FALSE) {
     //get the file extension
    $ext = @end(explode('.', $filename));
    $ext = strtolower($ext);

    //check the array for a predetermined mime type. If not found, add a warning to the watch dog and use the system command to get a value
    if (!empty($this->mime_types[$ext])) 
	{
		if (TRUE === $debug) {
			return array('mime_type' => $this->mime_types[$ext], 'method' => 'from_array');
		}
			return $this->mime_types[$ext];
    }
	else
	{
		//this file was not in the drupal mime system.
		//add a line to the watchdog informing us of this incident
		watchdog('MimeClass', 'Encountered a file extension not covered in the drupal mime system:'.$ext);		
	}

	//by default return application octet stream
	if (TRUE == $debug) {
		return array('mime_type' => 'application/octet-stream', 'method' => 'last_resort');
	}
    return 'application/octet-stream';
  }

  /**
   * function: get_extension
   * description: returns *one* valid file extension for a given $mime_type
   *
   * @param type $mime_type
   * @param type $debug
   * @return type
   */
  public function get_extension($type, $debug = FALSE) {

    if (!empty($this->ext_types[$type])) 
	{
	  if (TRUE == $debug) {
			return array('extension' => $this->ext_types[$type], 'method' => 'from_array');
      }
	  return $this->ext_types[$type];		
	}
	else
	{
		//return bin by default... this could cause errors if handled incorrectly and probably should return false or -1 or something to be handled client side.
		if (TRUE == $debug) {
			return array('extension' => 'bin', 'method' => 'last_resort');
		}		
		return 'bin';
	}
}

}

