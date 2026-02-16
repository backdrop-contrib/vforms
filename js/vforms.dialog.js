/**
 * commit concurrency: 401
 */
(function ($) {
  Backdrop.behaviors.vformsDialogFix = {
    attach: function (context, settings) {
      if (!settings.vforms || !settings.vforms.keepFormActions) return;

      // If the dialog buttonset exists, remove it so form action buttons are used.
      $('.ui-dialog-buttonpane', context).once('vforms-dialog-fix').remove();
    }
  };
})(jQuery);

