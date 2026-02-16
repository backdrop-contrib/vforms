/**
 * commit concurrency: 401
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

(function ($, Backdrop) {
  "use strict";

  function vforms_apply_dialog_link($link, href, width) {
    if (!$link || !$link.length) return;

    $link.addClass('use-ajax');
    $link.attr('data-dialog', 'true');
    $link.attr('data-dialog-type', 'dialog');
    $link.attr('data-dialog-options', JSON.stringify({
      width: parseInt(width, 10) || 900,
      target: '#vforms-dialog-target',
      modal: false,
      draggable: true,
      resizable: true
    }));

    $link.attr('href', href);
  }

  function vforms_href_matches(href, targets) {
    if (!href) return false;
    // Normalize leading slash.
    var h = href;
    // Ignore fragment.
    if (h.indexOf('#') !== -1) {
      h = h.split('#')[0];
    }

    for (var i = 0; i < targets.length; i++) {
      var t = targets[i];
      if (!t) continue;

      if (h === t) return true;
      if (h === '/' + t) return true;

      // Allow query string.
      if (h.indexOf(t + '?') === 0) return true;
      if (h.indexOf('/' + t + '?') === 0) return true;
    }
    return false;
  }

  Backdrop.behaviors.vformsDialogLinks = {
    attach: function (context, settings) {
      settings = settings || {};
      if (!settings.vforms) return;

      var base = (Backdrop.settings && Backdrop.settings.basePath) ? Backdrop.settings.basePath : '/';

      // 1) Convert the Edit local task to open the configured variant in a dialog.
      var editCfg = settings.vforms.editTabDialog;
      if (editCfg && editCfg.enabled && editCfg.nid && editCfg.href) {
        var nid = editCfg.nid;
        var targets = [
          base + 'node/' + nid + '/edit',
          'node/' + nid + '/edit'
        ];

        var $links = $('ul.tabs.primary a', context);
        if ($.fn.once) {
          $links = $links.once('vforms-edit-tab-dialog');
        }
        else {
          $links = $links.filter(function () { return !$(this).data('vformsEditTabDialogBound'); })
                         .data('vformsEditTabDialogBound', true);
        }

        $links.each(function () {
          var $a = $(this);
          var href = $a.attr('href') || '';
          if (vforms_href_matches(href, targets)) {
            vforms_apply_dialog_link($a, editCfg.href, editCfg.width);
          }
        });
      }

      // 2) Convert node/add/<type> links on the node/add listing page.
      var createMap = settings.vforms.createDialogLinks;
      if (createMap) {
        var $aAll = $('a', context);
        if ($.fn.once) {
          $aAll = $aAll.once('vforms-create-dialog-links');
        }
        else {
          $aAll = $aAll.filter(function () { return !$(this).data('vformsCreateDialogLinksBound'); })
                       .data('vformsCreateDialogLinksBound', true);
        }

        $aAll.each(function () {
          var $a = $(this);
          var href = $a.attr('href') || '';

          // Try each bundle (small list; OK).
          for (var bundle in createMap) {
            if (!createMap.hasOwnProperty(bundle)) continue;
            var info = createMap[bundle];
            if (!info || !info.href) continue;

            var targets = [
              base + 'node/add/' + bundle,
              'node/add/' + bundle
            ];

            if (vforms_href_matches(href, targets)) {
              vforms_apply_dialog_link($a, info.href, info.width);
              break;
            }
          }
        });
      }
    }
  };

})(jQuery, Backdrop);
