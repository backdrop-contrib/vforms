/**
 * commit concurrency: 401
 */

(function ($, Backdrop) {
  "use strict";

  /**
   * Reload the page after a successful dialog save.
   *
   * Response format (from PHP):
   *   { command: 'vformsReload', destination: '/some/path' }
   */
  Backdrop.ajax.prototype.commands.vformsReload = function (ajax, response, status) {
    // If a destination is provided, navigate to it.
    if (response && response.destination) {
      window.location.href = response.destination;
      return;
    }

    // Otherwise, hard reload the current page.
    window.location.reload();
  };

})(jQuery, Backdrop);
