/**
 * commit concurrency: 302.
 */

(function ($) {

  function mfT(str) {
    return (window.Backdrop && typeof Backdrop.t === 'function') ? Backdrop.t(str) : str;
  }

  function mfOnce($elements, key) {
    if ($.fn.once) {
      return $elements.once(key);
    }
    // Fallback if once() isn't present.
    return $elements.filter(function () {
      var $el = $(this);
      var flag = 'vformsOnce_' + key;
      if ($el.data(flag)) {
        return false;
      }
      $el.data(flag, true);
      return true;
    });
  }

  Backdrop.behaviors.vformsVariantBulk = {
    attach: function (context) {
      // "Select / deselect all" master checkbox.
      mfOnce($('.vforms-select-all', context), 'vforms-select-all').each(function () {
        var $master = $(this);
        var tableId = $master.attr('data-table-id') || $master.data('table-id') || $master.data('tableId');
        if (!tableId) {
          return;
        }

        var $table = $('#' + tableId, context);
        if (!$table.length) {
          $table = $('#' + tableId);
        }

        function getBoxes() {
          return $table.find('input.vforms-include[type=checkbox]:enabled');
        }

        function syncMaster() {
          var $boxes = getBoxes();
          if (!$boxes.length) {
            $master.prop('checked', false);
            return;
          }
          var all = true;
          $boxes.each(function () {
            if (!this.checked) {
              all = false;
              return false;
            }
          });
          $master.prop('checked', all);
        }

        $master.on('change', function () {
          var checked = this.checked;
          getBoxes().each(function () {
            this.checked = checked;
          }).trigger('change');
        });

        // Keep master checkbox updated when any individual checkbox changes.
        mfOnce($table, 'vforms-table-bind-' + tableId)
          .on('change', 'input.vforms-include[type=checkbox]', syncMaster);

        syncMaster();
      });

      // Invert selection button.
      mfOnce($('.vforms-invert', context), 'vforms-invert').on('click', function (e) {
        e.preventDefault();
        var $btn = $(this);
        var tableId = $btn.attr('data-table-id') || $btn.data('table-id') || $btn.data('tableId');
        if (!tableId) {
          return;
        }

        var $table = $('#' + tableId, context);
        if (!$table.length) {
          $table = $('#' + tableId);
        }

        var $boxes = $table.find('input.vforms-include[type=checkbox]:enabled');
        $boxes.each(function () {
          this.checked = !this.checked;
        }).trigger('change');
      });
    }
  };

  // ----------------------------------------------------------------------
  // Variant grouping UI (admin-only).
  //
  // This is a UI convenience layer that:
  //  - Creates “group header” rows for existing groups.
  //  - Lets the user add a new group header via the select at the top.
  //  - Treats rows underneath a group header as members of that group.
  //  - Updates each component row’s hidden/visible group textbox before save.
  //
  // No runtime behavior changes: the saved variant still only stores
  // per-component (include, weight, group).
  // ----------------------------------------------------------------------

  var mfGroupUid = 0;

  function mfGetColspan($table) {
    var th = $table.find('thead th').length;
    if (th) return th;
    var td = $table.find('tbody tr:first td').length;
    return td || 1;
  }

  function mfMakeGroupRow(title, colspan, opts) {
    opts = opts || {};
    var isUngrouped = !!opts.ungrouped;
    var uid = ++mfGroupUid;
    var mode = opts.mode || 'open';

    var $tr = $('<tr class="vforms-group-row" />');
    $tr.attr('data-vforms-group-uid', uid);
    if (isUngrouped) {
      $tr.addClass('vforms-group-row-ungrouped');
    }
    var $td = $('<td class="vforms-group-cell" />').attr('colspan', colspan);

    var $bar = $('<div class="vforms-group-bar" />');
    var $label;
    var $title;
    var $collapse = $();
    if (isUngrouped) {
      $label = $('<span class="vforms-group-label" />').text(mfT('Ungrouped'));
      // No editable title for ungrouped regions.
      $title = $();
    }
    else {
      $label = $('<span class="vforms-group-label" />').text(mfT('Group:'));
      $title = $('<input type="text" class="vforms-group-title form-text" />').val(title);
      var radioName = 'vforms_group_collapse_' + uid;
      $collapse = $('<span class="vforms-group-collapse" />');

      function addOpt(val, label) {
        var $lbl = $('<label class="vforms-group-collapse-opt" />');
        var $r = $('<input type="radio" class="vforms-group-collapse-radio" />').attr({
          name: radioName,
          value: val
        });
        if (mode === val) {
          $r.prop('checked', true);
        }
        $lbl.append($r).append($('<span />').text(mfT(label)));
        $collapse.append($lbl);
      }

      addOpt('closed', 'closed');
      addOpt('open', 'open');
      addOpt('locked_open', 'locked open');
    }

    var $up = $('<a href="#" class="vforms-group-move vforms-group-move-up" />').attr({
      title: mfT('Move group up'),
      'aria-label': mfT('Move group up')
    }).text('▲');
    var $down = $('<a href="#" class="vforms-group-move vforms-group-move-down" />').attr({
      title: mfT('Move group down'),
      'aria-label': mfT('Move group down')
    }).text('▼');
    var $remove = $('<a href="#" class="vforms-group-remove" />').attr({
      title: mfT('Remove group header'),
      'aria-label': mfT('Remove group header')
    }).text('✕');

    $bar.append($label);
    if ($title && $title.length) {
      $bar.append($title);
    }
    if ($collapse && $collapse.length) {
      $bar.append($collapse);
    }
    $bar.append($up, $down, $remove);
    $td.append($bar);
    $tr.append($td);
    return $tr;
  }

  function mfIsGroupRow($tr) {
    return $tr.hasClass('vforms-group-row');
  }

  function mfIsUngroupedRow($tr) {
    return $tr.hasClass('vforms-group-row-ungrouped');
  }

  function mfComponentRows($table) {
    // Component rows are the standard draggable rows.
    return $table.find('tbody tr').filter(function () {
      var $tr = $(this);
      if (mfIsGroupRow($tr)) return false;
      // Backdrop tabledrag marks rows with class "draggable".
      if ($tr.hasClass('draggable')) return true;
      // Fallback: rows that contain a weight element.
      return $tr.find('select.vforms-weight').length > 0;
    });
  }

  function mfGetRowGroupValue($row) {
    var $g = $row.find('input.vforms-group-input');
    if (!$g.length) return '';
    return $.trim($g.val() || '');
  }

  function mfSetRowGroupValue($row, value) {
    var $g = $row.find('input.vforms-group-input');
    if (!$g.length) return;
    $g.val(value);
  }

  function mfGetRowFieldsetMode($row) {
    var $m = $row.find('input.vforms-group-fs-mode');
    if (!$m.length) return '';
    return $.trim($m.val() || '');
  }

  function mfSetRowFieldsetMode($row, value) {
    var $m = $row.find('input.vforms-group-fs-mode');
    if (!$m.length) return;
    $m.val(value);
  }

  function mfGetGroupRowFieldsetMode($groupRow) {
    var $checked = $groupRow.find('input.vforms-group-collapse-radio:checked');
    return $checked.length ? String($checked.val()) : 'open';
  }


  function mfRenumberWeights($table) {
    var i = 0;
    mfComponentRows($table).each(function () {
      var $row = $(this);
      var $w = $row.find('select.vforms-weight');
      if (!$w.length) return;

      // Keep within available options.
      if ($w.find('option[value="' + i + '"]').length) {
        $w.val(String(i));
      }
      else {
        // Fallback to closest available (usually 0..delta).
        $w.val($w.find('option:last').val());
      }
      i++;
    });
  }

  function mfUpdateGroupAssignments($table) {
    var current = '';
    var currentMode = 'open';
    var $tbody = $table.find('tbody');
    if (!$tbody.length) return;

    $tbody.children('tr').each(function () {
      var $tr = $(this);
      if (mfIsGroupRow($tr)) {
        if (mfIsUngroupedRow($tr)) {
          current = '';
          currentMode = 'open';
        }
        else {
          var v = $.trim($tr.find('input.vforms-group-title').val() || '');
          current = v;
          currentMode = mfGetGroupRowFieldsetMode($tr);
        }
        return;
      }

      if (current) {
        $tr.addClass('vforms-in-group');
        mfSetRowFieldsetMode($tr, currentMode);
      }
      else {
        $tr.removeClass('vforms-in-group');
        mfSetRowFieldsetMode($tr, '');
      }

      mfSetRowGroupValue($tr, current);
    });
  }


  function mfEnsureUngroupedHeaders($table) {
    var $tbody = $table.find('tbody');
    if (!$tbody.length) return;

    var colspan = mfGetColspan($table);
    var lastHeaderState = null; // null = start, '' = ungrouped, 'X' = grouped

    $tbody.children('tr').each(function () {
      var $tr = $(this);

      if (mfIsGroupRow($tr)) {
        lastHeaderState = mfIsUngroupedRow($tr) ? '' : 'grouped';
        return;
      }

      // Component row: if it is ungrouped (group value is ''), ensure there is
      // an Ungrouped header immediately above it.
      var g = mfGetRowGroupValue($tr);
      if (!g) {
        if (lastHeaderState !== '') {
          var $hdr = mfMakeGroupRow('', colspan, { ungrouped: true });
          $hdr.insertBefore($tr);
          lastHeaderState = '';
        }
      }
      else {
        // First grouped row after an ungrouped run: don't force a header here.
        lastHeaderState = 'grouped';
      }
    });
  }

  function mfNextGroupTitle($table) {
    var base = mfT('Group');
    var used = {};
    $table.find('tr.vforms-group-row input.vforms-group-title').each(function () {
      var v = $.trim($(this).val() || '');
      if (v) used[v] = true;
    });
    if (!used[base]) return base;
    for (var i = 2; i < 1000; i++) {
      var candidate = base + ' ' + i;
      if (!used[candidate]) return candidate;
    }
    return base + ' ' + (Math.floor(Math.random() * 9000) + 1000);
  }

  function mfBlockForGroupRow($groupRow) {
    var $block = $groupRow.nextUntil('tr.vforms-group-row');
    return $groupRow.add($block);
  }

  function mfMoveGroupBlock($groupRow, direction) {
    var $tbody = $groupRow.closest('tbody');
    if (!$tbody.length) return;

    var $block = mfBlockForGroupRow($groupRow);

    if (direction === 'up') {
      var $prev = $groupRow.prevAll('tr.vforms-group-row').first();
      if (!$prev.length) return;
      $block.insertBefore($prev);
    }
    else {
      var $next = $groupRow.nextAll('tr.vforms-group-row').first();
      if (!$next.length) return;

      var $nextEnd = $next.nextUntil('tr.vforms-group-row').last();
      if (!$nextEnd.length) $nextEnd = $next;
      $block.insertAfter($nextEnd);
    }
  }

  function mfBuildGroupRowsFromExisting($table) {
    // If group headers already exist (e.g., Ajax refresh), don't duplicate.
    if ($table.find('tbody tr.vforms-group-row').length) {
      return;
    }

    var $rows = mfComponentRows($table);
    if (!$rows.length) return;

    // Find groups in first-seen order.
    var order = [];
    $rows.each(function () {
      var g = mfGetRowGroupValue($(this));
      if (!g) return;
      if ($.inArray(g, order) === -1) order.push(g);
    });

    if (!order.length) return;

    var colspan = mfGetColspan($table);
    var $tbody = $table.find('tbody');

    // Rebuild as contiguous blocks: for each group, insert header at the first
    // occurrence and pull all rows for that group under it.
    $.each(order, function (_, g) {
      var $members = $rows.filter(function () {
        return mfGetRowGroupValue($(this)) === g;
      });
      if (!$members.length) return;

      var $first = $members.first();
      var mode = mfGetRowFieldsetMode($first) || 'open';
      var $hdr = mfMakeGroupRow(g, colspan, { mode: mode });
      $hdr.insertBefore($first);

      var $after = $hdr;
      $members.each(function () {
        var $m = $(this);
        $m.insertAfter($after);
        $after = $m;
      });
    });

    // Do not normalize here; caller will insert Ungrouped headers first, then
    // normalize assignments.
  }

  function mfAddGroupAtTop($table, title) {
    var $tbody = $table.find('tbody');
    if (!$tbody.length) return;

    var colspan = mfGetColspan($table);
    var $hdr = mfMakeGroupRow(title, colspan);
    $tbody.prepend($hdr);
  }

  function mfAddUngroupedAtTop($table) {
    var $tbody = $table.find('tbody');
    if (!$tbody.length) return;

    var colspan = mfGetColspan($table);
    var $hdr = mfMakeGroupRow('', colspan, { ungrouped: true });
    $tbody.prepend($hdr);
  }

  Backdrop.behaviors.vformsVariantGroups = {
    attach: function (context) {
      // Enhance any vforms components tables present.
      mfOnce($('table.vforms-components-table', context), 'vforms-groups-table').each(function () {
        var $table = $(this);
        mfBuildGroupRowsFromExisting($table);

        // Ensure "Ungrouped" headers exist anywhere ungrouped rows appear.
        mfEnsureUngroupedHeaders($table);

        // Normalize assignments based on group-header layout.
        mfUpdateGroupAssignments($table);

        // Update group textbox values after any tabledrag action.
        // (mouseup occurs after a drag completes)
        mfOnce($table, 'vforms-groups-tabledrag-listener')
          .on('mouseup', '.tabledrag-handle', function () {
            // Defer slightly to let tabledrag finish DOM operations.
            window.setTimeout(function () {
              mfUpdateGroupAssignments($table);
            }, 25);
          });

        // Group title edits.
        mfOnce($table, 'vforms-groups-title-listener')
          .on('input', 'input.vforms-group-title', function () {
            mfUpdateGroupAssignments($table);
          });

        // Group collapse mode changes.
        mfOnce($table, 'vforms-groups-collapse-listener')
          .on('change', 'input.vforms-group-collapse-radio', function () {
            mfUpdateGroupAssignments($table);
          });

        // Move group up/down.
        mfOnce($table, 'vforms-groups-move-listener')
          .on('click', 'a.vforms-group-move-up, a.vforms-group-move-down', function (e) {
            e.preventDefault();
            var $link = $(this);
            var $groupRow = $link.closest('tr.vforms-group-row');
            if (!$groupRow.length) return;

            mfMoveGroupBlock($groupRow, $link.hasClass('vforms-group-move-up') ? 'up' : 'down');
            mfRenumberWeights($table);
            mfUpdateGroupAssignments($table);
          });

        // Remove a group header (does not delete fields; simply ungroups rows).
        mfOnce($table, 'vforms-groups-remove-listener')
          .on('click', 'a.vforms-group-remove', function (e) {
            e.preventDefault();
            var $groupRow = $(this).closest('tr.vforms-group-row');
            if (!$groupRow.length) return;

            // For grouped headers, keep the rows ungrouped by inserting an
            // Ungrouped header in the same position.
            var wasUngrouped = mfIsUngroupedRow($groupRow);

            // Clear group assignment for contiguous member rows.
            var $members = $groupRow.nextUntil('tr.vforms-group-row');
            $members.each(function () {
              var $m = $(this);
              mfSetRowGroupValue($m, '');
              mfSetRowFieldsetMode($m, '');
            });

            if (wasUngrouped) {
              // Removing an ungrouped header merges this region into the
              // previous region.
              $groupRow.remove();
            }
            else {
              // Replace the group header with an ungrouped header.
              var colspan = mfGetColspan($table);
              var $uh = mfMakeGroupRow('', colspan, { ungrouped: true });
              $groupRow.replaceWith($uh);
            }

            mfUpdateGroupAssignments($table);
          });
      });

      // Add-group button (top of the Form components fieldset).
      // No prompt: add a group immediately and let the user rename inline.
      var $addBtns = $('.vforms-add-group-button', context);
      $addBtns = mfOnce($addBtns, 'vforms-add-group-btn');
      $addBtns.on('click', function (e) {
        e.preventDefault();
        e.stopPropagation();

        var $btn = $(this);
        var targetId = $btn.attr('data-target-table') || $btn.data('target-table');
        var $table = targetId ? $('#' + targetId) : $();
        if (!$table.length) {
          // Fallback to the first components table on the page.
          $table = $('table.vforms-components-table').first();
        }
        if (!$table.length) return false;

        var title = mfNextGroupTitle($table);
        mfAddGroupAtTop($table, title);

        // Make sure there's an Ungrouped region right after the newly added
        // group header, so adding a group doesn't automatically scoop up
        // existing rows.
        mfEnsureUngroupedHeaders($table);
        mfUpdateGroupAssignments($table);

        // Focus the new title field to encourage immediate rename.
        var $input = $table.find('tr.vforms-group-row:first input.vforms-group-title');
        if ($input.length) {
          $input.focus();
          $input.select();
        }

        return false;
      });

      // Add-ungrouped button.
      var $addUngroupedBtns = $('.vforms-add-ungrouped-button', context);
      $addUngroupedBtns = mfOnce($addUngroupedBtns, 'vforms-add-ungrouped-btn');
      $addUngroupedBtns.on('click', function (e) {
        e.preventDefault();
        e.stopPropagation();

        var $btn = $(this);
        var targetId = $btn.attr('data-target-table') || $btn.data('target-table');
        var $table = targetId ? $('#' + targetId) : $();
        if (!$table.length) {
          $table = $('table.vforms-components-table').first();
        }
        if (!$table.length) return false;

        mfAddUngroupedAtTop($table);
        mfUpdateGroupAssignments($table);
        return false;
      });

      // Before save, ensure group textbox values match the group-header layout.
      mfOnce($('form', context), 'vforms-groups-on-submit').on('submit', function () {
        var $form = $(this);
        $form.find('table.vforms-components-table').each(function () {
          var $table = $(this);
          mfUpdateGroupAssignments($table);
          // Optional: normalize weights so server-side save reflects table order.
          mfRenumberWeights($table);
        });
      });
    }
  };

})(jQuery);
