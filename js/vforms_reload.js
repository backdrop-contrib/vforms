/**
 * commit concurrency: 401
 */

(function ($, Backdrop) {
  Backdrop.ajax.prototype.commands.vformsReload = function (ajax, response, status) {
    window.location.reload();
  };
})(jQuery, Backdrop);
