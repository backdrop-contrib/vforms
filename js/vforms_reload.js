/**
 * commit concurrency: 300.
 */

(function ($, Backdrop) {
  Backdrop.ajax.commands.vformsReload = function (ajax, response, status) {
    window.location.reload();
  };
})(jQuery, Backdrop);
