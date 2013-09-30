/**
 * @file
 * Triggers the display of a spinning icon when the form is submitted.
 */
(function ($) {

  Drupal.behaviors.spinner = {
    attach: function(context, settings) {
      // Store what triggered the submit.
      $('form').once('submit-resolver', function() {
        $(this).click(function(event) {
          $(this).data('clicked', $(event.target));
        });
        $(this).keypress(function(event) {
          // On enter the first submit button is assumed as is most often the
          // case and this is part of the HTML 5 specification, although some
          // Browsers may choose the button with the lowest tab-index.
          if (event.which == 13) {
            $(this).data('clicked', $(':submit', this).first());
          }
        });
      });
      for (var base in settings.spinner) {
        var id = '#' + base;
        $(id, context).once('spinner', function () {
          var spinner = new Spinner(settings.spinner[base].opts);
          $(id).parents('form').one('submit', function(event) {
            if ($(this).data('clicked').is(id)) {
              event.preventDefault();
              // Add Message.
              var message = $('<div/>').text(settings.spinner[base].message);
              $(id).after(message);
              // Make UI changes.
              spinner.spin(this);
              $('#edit-next').hide();
              $('#edit-prev').hide(); 
              // Submit the form after a set timeout, this handles problems with
              // safari, in that safari submit's immediately..
              if (navigator.userAgent.indexOf('Safari') != -1 && navigator.userAgent.indexOf('Chrome') == -1) { 
                $(':submit').attr('disabled', 'disabled');
              }
              setTimeout(function() {
                // Allow for the button to be clicked, then click it then
                // prevent the default behavoir.
                $(id).removeAttr('disabled')
                  .click()
                  .click(function(event) {
                    event.preventDefault();
                  });
              }, 500);
            }
            return true;
          });
        });
      }
    }
  };
})(jQuery);
