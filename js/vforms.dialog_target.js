/**
 * commit concurrency: 300.
 */

(function ($, Backdrop) {
  Backdrop.behaviors.vformsDialogTarget = {
    attach: function (context) {
      // Only once per page.
      if ($('#vforms-dialog-target').length) return;

      // Append to BODY so it isn't constrained by Views wrappers.
      $('body').append('<div id="vforms-dialog-target" style="display:none;"></div>');
    }
  };
})(jQuery, Backdrop);

(function ($, Backdrop) {

  function forceFloaty($content) {
    if (!$content.length) return;

    // If the dialog was created, it will have uiDialog data.
    if ($content.data('uiDialog')) {
      // Force the options you want.
      $content.dialog('option', 'modal', false);
      $content.dialog('option', 'draggable', true);
      $content.dialog('option', 'resizable', true);

      // Optional: ensure it has a titlebar (drag handle).
      $content.closest('.ui-dialog')
        .find('.ui-dialog-titlebar')
        .css('cursor', 'move');
    }
  }

  Backdrop.behaviors.vformsForceFloaty = {
    attach: function (context) {
      // When the link is clicked, the dialog is created asynchronously.
      // So we watch for the dialog wrapper to appear.
      var $ctx = $(context);

      // Run now (in case it's already there).
      forceFloaty($('#vforms-dialog-target'));

      // Run again shortly after (covers the "flash then override").
      setTimeout(function () {
        forceFloaty($('#vforms-dialog-target'));
      }, 0);

      setTimeout(function () {
        forceFloaty($('#vforms-dialog-target'));
      }, 50);
    }
  };

})(jQuery, Backdrop);