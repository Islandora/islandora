(function (Drupal, $) {
  var ImageName = $('input[id=edit-name-0-value]').val();
  $(document).ajaxSuccess(function () {
    $('input[data-drupal-selector=edit-field-media-image-0-alt]').val(ImageName);
  });
})(Drupal, jQuery);
