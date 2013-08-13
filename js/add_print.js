/**
 * @file
* JavaScript file responsable for the print button behaviour.
* 
* The print button is added automatically to every view, as metadata
* can be printed from every object.
*
*/
(function ($) {
  $(document).ready(function() {
    $('.tabs .primary').append('<img id="print_btn" title="Print" src="/' + Drupal.settings.islandora.print_img + '"></img>');
    $('#print_btn').css("cursor","pointer");
    $('#print_btn').click(function() {
      window.location=Drupal.settings.islandora.print_link;
    });
  });
})(jQuery); 
  
