<?php

/**
 * @file
 * Admin UI for Node Form Variants.
 * commit concurrency: 401
 */

function vforms_admin_overview($node_type) {
  return backdrop_get_form('vforms_admin_overview_form', $node_type);
}

function vforms_admin_overview_form($form, &$form_state, $node_type) {
  $bundle = vforms_bundle_from_node_type($node_type);
  if ($bundle === '') {
    return array('#markup' => t('Could not determine content type.'));
  }

  // Ensure dropbutton JS/CSS is present for the Operations column.
  backdrop_add_library('system', 'dropbutton');

  // Ensure a stored "default" exists as a baseline.
  vforms_variant_get('node', $bundle, 'default');

  $variants = vforms_variants_all('node', $bundle);
  $settings = vforms_bundle_settings_get('node', $bundle);

  $variant_options = array();
  foreach ($variants as $key => $variant) {
    $label = !empty($variant['label']) ? $variant['label'] : $key;
    $variant_options[$key] = $label . ' (' . $key . ')';
  }

  // Current settings.
  $edit_enabled = !empty($settings['edit_tab']['enabled']) ? 1 : 0;
  $edit_variant = !empty($settings['edit_tab']['variant']) ? (string) $settings['edit_tab']['variant'] : 'default';
  $edit_dialog = !empty($settings['edit_tab']['dialog']) ? 1 : 0;
  $edit_dialog_width = !empty($settings['edit_tab']['dialog_width']) ? (int) $settings['edit_tab']['dialog_width'] : 900;

  $create_enabled = !empty($settings['create']['enabled']) ? 1 : 0;
  $create_variant = !empty($settings['create']['variant']) ? (string) $settings['create']['variant'] : 'default';
  $create_dialog = !empty($settings['create']['dialog']) ? 1 : 0;
  $create_dialog_width = !empty($settings['create']['dialog_width']) ? (int) $settings['create']['dialog_width'] : 900;

  $form['bundle'] = array(
    '#type' => 'value',
    '#value' => $bundle,
  );

  $form['help'] = array(
    '#markup' => '<p>' . t('Form variants let you show/hide and reorder fields (and some extra elements) on the node edit form. The table below lists all variants configured for this content type. Use the "Configure" link to edit a variant, or "Clone" to create a new variant based on an existing one.') . '</p>'
      . '<hr><p>' . t('The Variant routing settings let you select a variant to use for the standard Edit tab and node creation form, and optionally open those in a modal dialog.') . '</p>'
      . '<p>' . t('The following paths are available for direct linking to a specific variant:') . '<ul><li>'
      . t('Edit node:&emsp; `node/{nid}/form/{variant_key}`') . '</li><li>' 
      . t('Create node: `node/add/{content_type}/form/{variant_key}`') . '</li></ul>'
      . t('&emsp;&emsp;&emsp; where {variant_key} is the machine name of the variant you want to use (e.g. `default` or `quick`).')
      . '<p>' . t('DEVEL NOTE: Not all of the Variant routing functionality (below) works yet. Work is ongoing...') . '</p>',
  );

  $form['behavior'] = array(
    '#type' => 'fieldset',
    '#title' => t('Variant routing'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['behavior']['use_edit_tab_variant'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use variant form for Local tasks Edit button'),
    '#default_value' => $edit_enabled,
  );

  $form['behavior']['edit_tab_variant'] = array(
    '#type' => 'select',
    '#title' => t('Variant to use for Edit tab'),
    '#options' => $variant_options,
    '#default_value' => $edit_variant,
    '#states' => array(
      'visible' => array(
        ':input[name="use_edit_tab_variant"]' => array('checked' => TRUE),
      ),
    ),
  );

  
  $form['behavior']['edit_tab_dialog'] = array(
    '#type' => 'checkbox',
    '#title' => t('Open Edit tab in popup dialog'),
    '#default_value' => $edit_dialog,
    '#states' => array(
      'visible' => array(
        ':input[name="use_edit_tab_variant"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['behavior']['edit_tab_dialog_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Edit dialog width'),
    '#default_value' => $edit_dialog_width,
    '#size' => 6,
    '#states' => array(
      'visible' => array(
        ':input[name="use_edit_tab_variant"]' => array('checked' => TRUE),
        ':input[name="edit_tab_dialog"]' => array('checked' => TRUE),
      ),
    ),
    '#description' => t('Width in pixels.'),
  );

$form['behavior']['use_create_variant'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use variant form for creating new content'),
    '#default_value' => $create_enabled,
  );

  $form['behavior']['create_variant'] = array(
    '#type' => 'select',
    '#title' => t('Variant to use for Create form'),
    '#options' => $variant_options,
    '#default_value' => $create_variant,
    '#states' => array(
      'visible' => array(
        ':input[name="use_create_variant"]' => array('checked' => TRUE),
      ),
    ),
    '#description' => t('This affects the standard path node/add/@type.', array('@type' => $bundle)),
  );

  
  $form['behavior']['create_dialog'] = array(
    '#type' => 'checkbox',
    '#title' => t('Open Create form in popup dialog'),
    '#default_value' => $create_dialog,
    '#states' => array(
      'visible' => array(
        ':input[name="use_create_variant"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['behavior']['create_dialog_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Create dialog width'),
    '#default_value' => $create_dialog_width,
    '#size' => 6,
    '#states' => array(
      'visible' => array(
        ':input[name="use_create_variant"]' => array('checked' => TRUE),
        ':input[name="create_dialog"]' => array('checked' => TRUE),
      ),
    ),
    '#description' => t('Width in pixels.'),
  );

$form['behavior']['note'] = array(
    '#markup' => '<div class="description">' . t('Note: Non-default variants require the permission "Use non-default form variants".') . '</div>',
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );

  // Variants table.
  $rows = array();
  foreach ($variants as $key => $variant) {
    $label = !empty($variant['label']) ? $variant['label'] : $key;

    // Dropbutton links.
    $links = array(
      'configure' => array(
        'title' => t('Configure'),
        'href'  => "admin/structure/types/manage/$bundle/vforms/$key/edit",
      ),
      'clone' => array(
        'title' => t('Clone variant'),
        'href'  => "admin/structure/types/manage/$bundle/vforms/add",
        'query' => array('clone' => $key),
      ),
    );
    if ($key !== 'default') {
      $links['delete'] = array(
        'title' => t('Delete'),
        'href'  => "admin/structure/types/manage/$bundle/vforms/$key/delete",
      );
    }

    $drop = array(
      '#type' => 'dropbutton',
      '#links' => $links,
    );

    $rows[] = array(
      check_plain($label),
      check_plain($key),
      backdrop_render($drop),
    );
  }

  $form['variants_table'] = array(
    '#markup' => theme('table', array(
      'header' => array(t('Label'), t('Machine name'), t('Operations')),
      'rows' => $rows,
    )),
  );

  return $form;
}

function vforms_admin_overview_form_submit($form, &$form_state) {
  $bundle = (string) $form_state['values']['bundle'];

  $settings = array(
    'edit_tab' => array(
      'enabled' => !empty($form_state['values']['use_edit_tab_variant']) ? 1 : 0,
      'variant' => !empty($form_state['values']['edit_tab_variant']) ? (string) $form_state['values']['edit_tab_variant'] : 'default',
      'dialog' => !empty($form_state['values']['edit_tab_dialog']) ? 1 : 0,
      'dialog_width' => !empty($form_state['values']['edit_tab_dialog_width']) ? (int) $form_state['values']['edit_tab_dialog_width'] : 900,
    ),
    'create' => array(
      'enabled' => !empty($form_state['values']['use_create_variant']) ? 1 : 0,
      'variant' => !empty($form_state['values']['create_variant']) ? (string) $form_state['values']['create_variant'] : 'default',
      'dialog' => !empty($form_state['values']['create_dialog']) ? 1 : 0,
      'dialog_width' => !empty($form_state['values']['create_dialog_width']) ? (int) $form_state['values']['create_dialog_width'] : 900,
    ),
  );

  vforms_bundle_settings_set('node', $bundle, $settings);
  backdrop_set_message(t('Settings saved.'), 'status');

  $form_state['redirect'] = current_path();
}

function vforms_admin_variant_form($form, &$form_state, $node_type, $variant_key = NULL) {
  $bundle = vforms_bundle_from_node_type($node_type);

  // JS helpers for bulk selection controls.
  backdrop_add_library('vforms', 'admin_variant');

  // Lightweight JS-only template for the injected group header row.
  // UI-only: no server-side grouping logic is introduced.
  $group_row_tpl = '<tr class="vforms-group-row"><td colspan="__COLSPAN__"><span class="vforms-group-title-text">__TITLE_ESC__</span></td></tr>';
  backdrop_add_js(array('vforms' => array('group_row_tpl' => $group_row_tpl)), 'setting');

  // Draggable behavior is attached by theme_vforms_components_table() via
  // backdrop_add_tabledrag().

  $is_new = ($variant_key === NULL);
  $variants = vforms_variants_all('node', $bundle);

  $clone_key = '';
  $is_clone = FALSE;
  $clone_src = NULL;

  if ($is_new) {
    $clone_key = isset($_GET['clone']) ? (string) $_GET['clone'] : '';
    if ($clone_key !== '' && !empty($variants[$clone_key]) && is_array($variants[$clone_key])) {
      $is_clone = TRUE;
      $clone_src = $variants[$clone_key];
    }

    $base = $is_clone
      ? $clone_src
      : (!empty($variants['default']) ? $variants['default'] : vforms_variant_build_default('node', $bundle));

    $base_label = $is_clone
      ? (!empty($clone_src['label']) ? $clone_src['label'] : $clone_key)
      : '';

    $default_admin_mode = $is_clone
      ? vforms_variant_admin_extras_mode($base)
      : 'exclude';

    $variant = array(
      'label' => $is_clone ? t('Clone of @label', array('@label' => $base_label)) : '',
      'settings' => array(
        // New variants default to hiding admin sections; clones inherit.
        'admin_extras' => $default_admin_mode,
      ),
      'components' => $base['components'],
    );
  }
  else {
    $variant = vforms_variant_get('node', $bundle, $variant_key);
    if (!$variant) {
      $variant = vforms_variant_build_default('node', $bundle);
    }
  }

  // How to treat administrative "extra" sections on the node form.
  $admin_extras_mode = vforms_variant_admin_extras_mode($variant);
  $show_admin_section = ($admin_extras_mode === 'allow') ? 1 : 0;

  $form['bundle'] = array('#type' => 'value', '#value' => $bundle);
  $form['variant_key'] = array('#type' => 'value', '#value' => $variant_key);

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#required' => TRUE,
    '#default_value' => isset($variant['label']) ? $variant['label'] : '',
  );

  // For new variants, show a machine_name element that auto-generates from the
  // label (with an Edit toggle), matching the core field UI behavior.
  if ($is_new) {
    $form['new_key'] = array(
      '#type' => 'machine_name',
      '#title' => t('Machine name'),
      '#required' => TRUE,
      '#default_value' => '',
      '#machine_name' => array(
        'source' => array('label'),
        'exists' => 'vforms_variant_exists',
      ),
      '#description' => t('Lowercase letters, numbers, and underscores only. Example: quick'),
    );
  }

  if (!$is_new) {
    $form['machine_name'] = array(
      '#type' => 'item',
      '#title' => t('Machine name'),
      '#markup' => check_plain($variant_key),
    );
  }

  $form['show_admin_section'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show administrative section'),
    '#default_value' => $show_admin_section,
    '#description' => t('Shows the administrative fieldsets typically found under the "Additional settings" 
                         vertical tabs (Publishing options, Authoring information, URL path settings, etc.).'),
  );

  $form['components'] = array(
    '#type' => 'fieldset',
    '#title' => t('Form components'),
    '#description' => 'Choose which components appear on this form and drag to reorder. You may define variant groups, 
                       then drag fields into them. These Groups a)are not shared between variants, b)may override
                       Field group settings for this form only. Results may vary.',
    '#tree' => TRUE,
  );

  // UI-only helper: create draggable group headers in the components table.
  // This is purely presentational; it does not alter Node Form Variants behavior.
  $form['components']['add_group'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('vforms-add-group-control')),
    '#weight' => -50,
  );
  $form['components']['add_group']['add_button'] = array(
    '#type' => 'markup',
    '#markup' => '<a href="#" class="button vforms-add-group-button" data-target-table="vforms-components-table">' . t('Add variant group') . '</a>',
  );
  $form['components']['add_group']['add_ungrouped_button'] = array(
    '#type' => 'markup',
    '#markup' => ' <a href="#" class="button vforms-add-ungrouped-button" data-target-table="vforms-components-table">' . t('Add ungrouped region') . '</a>',
  );
  $form['components']['add_group']['help'] = array(
    '#markup' => '<div class="description">' . t('Add group/region headers, then drag components underneath to assign them. 
                                                  Ungrouped regions in this form will generally follow Field Group settings 
                                                  defined at "Manage fields".') . '</div>',
  );

  // Bulk selection controls for the main components table.
  $form['components']['bulk_controls'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('vforms-bulk-controls')),
  );
  $form['components']['bulk_controls']['select_all'] = array(
    '#type' => 'checkbox',
    '#title' => t('Select / deselect all'),
    '#attributes' => array(
      'class' => array('vforms-select-all'),
      'data-table-id' => 'vforms-components-table',
    ),
  );
  $form['components']['bulk_controls']['invert'] = array(
    '#markup' => ' <a href="#" class="invert-button vforms-invert" data-table-id="vforms-components-table">' . t('Invert selection') . '</a>',
  );

  $header = array(
    'include' => t('Include'),
    'label' => t('Component'),
    'name'    => t('Machine name'),
    'kind' => t('Type'),
    'weight' => t('Weight'),
    'group' => array('data' => t('Group'), 'class' => array('vforms-group-col'), 'style' => 'display:none'),
  );

  // Backdrop does not provide a native Form API element type "table".
  // Instead, we theme this container as a table.
  $form['components']['table'] = array(
    '#theme' => 'vforms_components_table',
    '#header' => $header,
    '#attributes' => array(
      'id' => 'vforms-components-table',
      'class' => array('vforms-components-table'),
    ),
  );

  // Administrative components are always separate from (and follow) the main
  // form fields, so present them in their own table.
  $form['admin_components'] = array(
    '#type' => 'fieldset',
    '#title' => t('Administrative section'),
    '#description' => t('These components normally appear after all fields under the "Additional settings" vertical tabs.'),
    '#tree' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="show_admin_section"]' => array('checked' => TRUE),
      ),
    ),
  );

  // Bulk selection controls for the administrative components table.
  $form['admin_components']['bulk_controls'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('vforms-bulk-controls')),
    '#states' => $form['admin_components']['#states'],
  );
  $form['admin_components']['bulk_controls']['select_all'] = array(
    '#type' => 'checkbox',
    '#title' => t('Select / deselect all'),
    '#attributes' => array(
      'class' => array('vforms-select-all'),
      'data-table-id' => 'vforms-admin-components-table',
    ),
  );
  $form['admin_components']['bulk_controls']['invert'] = array(
    '#markup' => ' <a href="#" class="invert-button vforms-invert" data-table-id="vforms-admin-components-table">' . t('Invert selection') . '</a>',
  );

  $form['admin_components']['table'] = array(
    '#theme' => 'vforms_components_table',
    '#header' => $header,
    '#attributes' => array(
      'id' => 'vforms-admin-components-table',
    ),
  );

  $components = vforms_components_list('node', $bundle);

  foreach ($components as $ckey => $meta) {
    if (empty($variant['components'][$ckey]) || !is_array($variant['components'][$ckey])) {
      $variant['components'][$ckey] = array('include' => 1, 'weight' => 0, 'group' => '', 'fieldset_mode' => 'open');
    }
    // Backwards-compat: older configs won't have this key.
    elseif (!isset($variant['components'][$ckey]['fieldset_mode'])) {
      $variant['components'][$ckey]['fieldset_mode'] = 'open';
    }
  }

  // New variants default to hiding administrative extras. Initialize those
  // admin extras as unchecked, so turning the admin section on later starts
  // from a clean slate.
  if ($is_new && !$is_clone) {
    foreach ($components as $ckey => $meta) {
      if ($meta['kind'] === 'extra' && $meta['name'] !== 'title') {
        $variant['components'][$ckey]['include'] = 0;
      }
    }
  }

  $main_keys = array();
  $admin_keys = array();
  foreach ($components as $ckey => $meta) {
    $is_admin_extra = ($meta['kind'] === 'extra' && $meta['name'] !== 'title');
    if ($is_admin_extra) {
      $admin_keys[] = $ckey;
    }
    else {
      $main_keys[] = $ckey;
    }
  }

  $sort_by_weight = function ($a, $b) use ($variant) {
    $wa = isset($variant['components'][$a]['weight']) ? (int) $variant['components'][$a]['weight'] : 0;
    $wb = isset($variant['components'][$b]['weight']) ? (int) $variant['components'][$b]['weight'] : 0;
    if ($wa === $wb) return strcmp($a, $b);
    return ($wa < $wb) ? -1 : 1;
  };

  usort($main_keys, $sort_by_weight);
  usort($admin_keys, $sort_by_weight);

  // Main components table.
  foreach ($main_keys as $ckey) {
    $meta = $components[$ckey];
    $cfg  = $variant['components'][$ckey];

    $form['components']['table'][$ckey]['#attributes']['class'][] = 'draggable';

    $form['components']['table'][$ckey]['include'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($cfg['include']) ? 1 : 0,
      '#attributes' => array('class' => array('vforms-include')),
    );

    $form['components']['table'][$ckey]['label'] = array(
      '#markup' => check_plain($meta['label']),
    );

    $form['components']['table'][$ckey]['kind'] = array(
      '#markup' => ($meta['kind'] === 'field') ? t('Field') : t('Extra'),
    );

    $form['components']['table'][$ckey]['weight'] = array(
      '#type' => 'weight',
      '#default_value' => isset($cfg['weight']) ? (int) $cfg['weight'] : 0,
      '#attributes' => array('class' => array('vforms-weight')),
    );

    $form['components']['table'][$ckey]['group'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($cfg['group']) ? $cfg['group'] : '',
      '#attributes' => array('class' => array('vforms-group-input')),
      '#wrapper_attributes' => array(
        'class' => array('vforms-group-col'),
        'style' => 'display:none',
      ),
    );
    $form['components']['table'][$ckey]['fieldset_mode'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($cfg['fieldset_mode']) ? (string) $cfg['fieldset_mode'] : 'open',
      '#attributes' => array('class' => array('vforms-group-fs-mode')),
      '#wrapper_attributes' => array(
        'style' => 'display:none',
      ),
    );
  }

  // Admin components table.
  foreach ($admin_keys as $ckey) {
    $meta = $components[$ckey];
    $cfg  = $variant['components'][$ckey];

    $form['admin_components']['table'][$ckey]['#attributes']['class'][] = 'draggable';

    $form['admin_components']['table'][$ckey]['include'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($cfg['include']) ? 1 : 0,
      '#attributes' => array('class' => array('vforms-include')),
    );

    $form['admin_components']['table'][$ckey]['label'] = array(
      '#markup' => check_plain($meta['label']),
    );

    $form['admin_components']['table'][$ckey]['kind'] = array(
      '#markup' => t('Extra'),
    );

    $form['admin_components']['table'][$ckey]['weight'] = array(
      '#type' => 'weight',
      '#default_value' => isset($cfg['weight']) ? (int) $cfg['weight'] : 0,
      '#attributes' => array('class' => array('vforms-weight')),
    );

    $form['admin_components']['table'][$ckey]['group'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($cfg['group']) ? $cfg['group'] : '',
      '#attributes' => array('class' => array('vforms-group-input')),
      '#wrapper_attributes' => array(
        'class' => array('vforms-group-col'),
        'style' => 'display:none',
      ),
    );
    $form['admin_components']['table'][$ckey]['fieldset_mode'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($cfg['fieldset_mode']) ? (string) $cfg['fieldset_mode'] : 'open',
      '#attributes' => array('class' => array('vforms-group-fs-mode')),
      '#wrapper_attributes' => array(
        'style' => 'display:none',
      ),
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => $is_new ? t('Create variant') : t('Save changes'),
  );

  return $form;
}
function vforms_validate_machine_name($element, &$form_state) {
  $v = (string) $form_state['values']['new_key'];
  if (!preg_match('/^[a-z0-9_]+$/', $v)) {
    form_error($element, t('Machine name must contain only lowercase letters, numbers, and underscores.'));
  }
  if ($v === 'default') {
    form_error($element, t('"default" is reserved.'));
  }
}

function vforms_admin_variant_form_submit($form, &$form_state) {
  $bundle = $form_state['values']['bundle'];
  $variant_key = $form_state['values']['variant_key'];

  if (empty($variant_key)) {
    $variant_key = (string) $form_state['values']['new_key'];
  }

  $show_admin_section = !empty($form_state['values']['show_admin_section']);

  // If editing an existing variant, keep its component settings for keys that
  // may not be present in the current form (defensive).
  $existing = vforms_variant_get('node', $bundle, $variant_key);
  $existing_components = (!empty($existing['components']) && is_array($existing['components'])) ? $existing['components'] : array();

  $variant = array(
    'label' => (string) $form_state['values']['label'],
    'settings' => array(
      // Stored setting used at render time: hide all admin extras unless the
      // admin section is explicitly enabled for this variant.
      'admin_extras' => $show_admin_section ? 'allow' : 'exclude',
    ),
    'components' => array(),
  );

  $tables = array();
  if (!empty($form_state['values']['components']['table']) && is_array($form_state['values']['components']['table'])) {
    $tables[] = $form_state['values']['components']['table'];
  }
  if (!empty($form_state['values']['admin_components']['table']) && is_array($form_state['values']['admin_components']['table'])) {
    $tables[] = $form_state['values']['admin_components']['table'];
  }
  // Defensive fallback for older form structures.
  if (empty($tables) && !empty($form_state['values']['table']) && is_array($form_state['values']['table'])) {
    $tables[] = $form_state['values']['table'];
  }

  foreach ($tables as $rows) {
    foreach ($rows as $key => $row) {
      if (!is_array($row)) {
        continue;
      }
      $variant['components'][$key] = array(
        'include' => !empty($row['include']) ? 1 : 0,
        'weight'  => isset($row['weight']) ? (int) $row['weight'] : 0,
        'group'   => isset($row['group']) ? (string) $row['group'] : '',
        'fieldset_mode' => isset($row['fieldset_mode']) ? (string) $row['fieldset_mode'] : '',
      );
    }
  }

  // Preserve any prior component keys that were not present in this form.
  foreach ($existing_components as $k => $cfg) {
    if (!isset($variant['components'][$k]) && is_array($cfg)) {
      $variant['components'][$k] = $cfg;
    }
  }

  vforms_variant_set('node', $bundle, $variant_key, $variant);

  backdrop_set_message(t('Saved form variant %key.', array('%key' => $variant_key)));
  $form_state['redirect'] = "admin/structure/types/manage/$bundle/vforms";
}

function vforms_admin_variant_delete_form($form, &$form_state, $node_type, $variant_key) {
  $bundle = vforms_bundle_from_node_type($node_type);

  $form['bundle'] = array('#type' => 'value', '#value' => $bundle);
  $form['variant_key'] = array('#type' => 'value', '#value' => $variant_key);

  return confirm_form(
    $form,
    t('Delete form variant %key?', array('%key' => $variant_key)),
    "admin/structure/types/manage/$bundle/vforms",
    t('This cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function vforms_admin_variant_delete_form_submit($form, &$form_state) {
  $bundle = $form_state['values']['bundle'];
  $variant_key = $form_state['values']['variant_key'];

  if ($variant_key === 'default') {
    backdrop_set_message(t('The default variant cannot be deleted.'), 'warning');
  }
  else {
    vforms_variant_delete('node', $bundle, $variant_key);
    backdrop_set_message(t('Deleted form variant %key.', array('%key' => $variant_key)));
  }

  $form_state['redirect'] = "admin/structure/types/manage/$bundle/vforms";
}

function vforms_variant_exists($value) {
  $value = (string) $value;
  if ($value === '' || $value === 'default') {
    return TRUE;
  }

  // Determine bundle from the current path: /admin/structure/types/manage/{bundle}/...
  $args = arg();
  $bundle = isset($args[4]) ? $args[4] : '';
  if ($bundle === '') {
    return FALSE;
  }

  $variants = vforms_variants_all('node', $bundle);
  return !empty($variants[$value]);
}

function vforms_bundle_from_node_type($node_type) {
  // Backdrop may pass either a string (machine name) or an object.
  if (is_object($node_type) && !empty($node_type->type)) {
    return $node_type->type;
  }
  return (string) $node_type;
}
