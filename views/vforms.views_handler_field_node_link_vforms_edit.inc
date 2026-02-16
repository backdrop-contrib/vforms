<?php
/**
 * @file
 * Views field handler: Node Form Variants edit link.
 * commit concurrency: 401
 */

// Ensure the core node edit-link handler is available before we extend it.
include_once backdrop_get_path('module', 'node') . '/views/views_handler_field_node_link_edit.inc';

/**
 * Provides a node edit link that targets a Node Form Variants variant and can open
 * in a Backdrop modal dialog.
 */
class vforms_views_handler_field_node_link_vforms_edit extends views_handler_field_node_link_edit {

  protected function vforms_scalar($value, $default = '') {
    if (is_array($value)) {
      $value = reset($value);
    }
    if (is_object($value)) {
      // Don't ever pass objects into string functions.
      $value = $default;
    }
    if ($value === NULL) {
      $value = $default;
    }
    return (string) $value;
  }

  /**
   * Expands icon placeholders in a user-entered label.
   *
   * Rules:
   *  - %     => icon('note-pencil')
   *  - \%    => literal %
   *  - %%    => literal %
   *
   * Returns HTML markup; caller must set $options['html'] = TRUE in l().
   */
  protected function vforms_expand_display_text_with_icon($raw, $icon_name = 'note-pencil') {
    $raw = $this->vforms_scalar($raw, '');

    $out = '';
    $buf = '';

    $len = strlen($raw);
    for ($i = 0; $i < $len; $i++) {
      $ch = $raw[$i];

      // \%  -> literal %
      if ($ch === '\\' && ($i + 1) < $len && $raw[$i + 1] === '%') {
        $buf .= '%';
        $i++;
        continue;
      }

      if ($ch === '%') {
        // %% -> literal %
        if (($i + 1) < $len && $raw[$i + 1] === '%') {
          $buf .= '%';
          $i++;
          continue;
        }

        // % -> icon
        if ($buf !== '') {
          $out .= check_plain($buf);
          $buf = '';
        }
        $out .= icon($icon_name);
        continue;
      }

      $buf .= $ch;
    }

    if ($buf !== '') {
      $out .= check_plain($buf);
    }

    return $out;
  }

  /**
   * Try to detect a single node bundle (content type) from the current view.
   * Returns machine name (e.g. 'job') or NULL if unknown / multiple.
   */
  protected function vforms_detect_single_bundle_from_view() {
    if (empty($this->view) || !is_object($this->view)) {
      return NULL;
    }

    $view = $this->view;

    // 1) Best source in the Views UI: resolved options from the current display handler.
    $filters = NULL;
    if (!empty($view->display_handler) && is_object($view->display_handler) && method_exists($view->display_handler, 'get_option')) {
      $filters = $view->display_handler->get_option('filters');
    }

    // 2) Fallback: look on current display, then default display.
    if (!is_array($filters)) {
      $display_id = !empty($view->current_display) ? $view->current_display : 'default';

      if (!empty($view->display[$display_id]->display_options['filters']) && is_array($view->display[$display_id]->display_options['filters'])) {
        $filters = $view->display[$display_id]->display_options['filters'];
      }
      elseif (!empty($view->display['default']->display_options['filters']) && is_array($view->display['default']->display_options['filters'])) {
        $filters = $view->display['default']->display_options['filters'];
      }
      else {
        return NULL;
      }
    }

    // Parse filters config for node.type restriction.
    foreach ($filters as $fid => $f) {
      if (!is_array($f)) continue;

      $table = isset($f['table']) ? $f['table'] : '';
      $field = isset($f['field']) ? $f['field'] : '';

      // "Content: Type" filter.
      if ($table !== 'node' || $field !== 'type') {
        continue;
      }

      $val = isset($f['value']) ? $f['value'] : NULL;

      $vals = array();
      if (is_string($val) && $val !== '') {
        $vals[] = $val;
      }
      elseif (is_array($val)) {
        // Common formats:
        //  - ['job' => 'job']
        //  - ['job' => 1]
        //  - [0 => 'job']
        foreach ($val as $k => $v) {
          if (is_string($v) && $v !== '') {
            $vals[] = $v;
          }
          elseif (is_string($k) && $k !== '' && !empty($v)) {
            $vals[] = $k;
          }
        }
      }

      $vals = array_values(array_unique(array_filter($vals, 'strlen')));

      return (count($vals) === 1) ? $vals[0] : NULL;
    }

    return NULL;
  }

  /**
   * Build select options for variants of a specific bundle.
   * Always includes 'default'. Uses variant label when available.
   */
  protected function vforms_variant_options_for_bundle($bundle) {
    $opts = array();

    if (!is_string($bundle) || $bundle === '') {
      return array('default' => t('Default'));
    }

    // Pull configured variants for this bundle.
    $variants = vforms_variants_all('node', $bundle);

    // Ensure default exists.
    if (!isset($variants['default'])) {
      $variants['default'] = array('label' => t('Default'));
    }

    foreach ($variants as $key => $def) {
      if (!is_string($key) || !preg_match('/^[a-z0-9_]+$/', $key)) {
        continue;
      }
      $label = (!empty($def['label']) && is_string($def['label'])) ? $def['label'] : $key;
      // Show label, but keep the machine key obvious.
      $opts[$key] = ($key === $label) ? $label : ($label . ' (' . $key . ')');
    }

    // Put default at the top.
    if (isset($opts['default'])) {
      $default = array('default' => $opts['default']);
      unset($opts['default']);
      $opts = $default + $opts;
    }

    return $opts;
  }

  protected function vforms_detect_bundles_from_view() {
    if (empty($this->view) || !is_object($this->view)) {
      return array();
    }

    $view = $this->view;
    $filters = NULL;

    if (!empty($view->display_handler) && is_object($view->display_handler) && method_exists($view->display_handler, 'get_option')) {
      $filters = $view->display_handler->get_option('filters');
    }
    if (!is_array($filters)) {
      return array();
    }

    foreach ($filters as $fid => $f) {
      if (!is_array($f)) continue;

      if (!isset($f['table'], $f['field'])) continue;
      if ($f['table'] !== 'node' || $f['field'] !== 'type') continue;

      $val = isset($f['value']) ? $f['value'] : NULL;
      $vals = array();

      if (is_string($val) && $val !== '') {
        $vals[] = $val;
      }
      elseif (is_array($val)) {
        foreach ($val as $k => $v) {
          if (is_string($v) && $v !== '') {
            $vals[] = $v;
          }
          elseif (is_string($k) && $k !== '' && !empty($v)) {
            $vals[] = $k;
          }
        }
      }

      $vals = array_values(array_unique(array_filter($vals, 'strlen')));
      return $vals;
    }

    return array();
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['vforms_variant'] = array('default' => 'default');
    $options['vforms_modal'] = array('default' => 0);
    $options['vforms_dialog_width'] = array('default' => 500);
    $options['vforms_variant_map'] = array('default' => array());

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Optional: add help to the existing "Text to display" field.
    if (isset($form['text']['#description'])) {
      $form['text']['#description'] .= ' ' . t('Leave this field blank to present only a note-pencil icon. 
                                                Use % to embed the icon within text. Escape with \\% or %% 
                                                for a literal percent sign.');
    }
    elseif (isset($form['text'])) {
      $form['text']['#description'] = t('Leave this field blank to present only a note-pencil icon. 
                                                Use % to embed the icon within text. Escape with \\% or %% 
                                                for a literal percent sign.');
    }

    $form['vforms_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Form variant settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    /**
     * IMPORTANT:
     * - Visually nested under $form['vforms_settings'][...]
     * - But #parents forces submitted values to remain flat:
     *     $form_state['values']['options']['vforms_variant']
     *     $form_state['values']['options']['vforms_modal']
     *     $form_state['values']['options']['vforms_dialog_width']
     *   so Views auto-saves into $this->options[...] exactly as before.
     */

    $bundles = $this->vforms_detect_bundles_from_view();
    $current_global = (string) $this->options['vforms_variant'];
    $current_map = (!empty($this->options['vforms_variant_map']) && is_array($this->options['vforms_variant_map']))
      ? $this->options['vforms_variant_map']
      : array();

    if (count($bundles) === 1) {
      // Single bundle: show dropdown of that bundle's variants.
      $bundle = $bundles[0];
      $variant_options = $this->vforms_variant_options_for_bundle($bundle);

      if ($current_global !== '' && !isset($variant_options[$current_global])) {
        $variant_options = array($current_global => t('@key (missing)', array('@key' => $current_global))) + $variant_options;
      }

      $form['vforms_settings']['vforms_variant'] = array(
        '#type' => 'select',
        '#title' => t('Form variant'),
        '#options' => $variant_options,
        '#default_value' => ($current_global !== '') ? $current_global : 'default',
        '#parents' => array('options', 'vforms_variant'),
      );
    }
    else {
      // Multiple (or unknown): keep a global variant name, plus per-type overrides.
      $form['vforms_settings']['vforms_variant'] = array(
        '#type' => 'textfield',
        '#title' => t('Form variant (global fallback)'),
        '#default_value' => $current_global,
        '#description' => t('Used when no per-type override is set.'),
        '#parents' => array('options', 'vforms_variant'),
      );

      if (count($bundles) > 1) {
        $form['vforms_settings']['vforms_variant_map'] = array(
          '#type' => 'fieldset',
          '#title' => t('Variants per content type'),
          '#collapsible' => TRUE,
          '#collapsed' => FALSE,
          '#description' => t('Overrides the global fallback for nodes of each content type in this view.'),
          '#attributes' => array( // a quick presentation hack...
            'style' => 'max-width:100%; overflow-x:auto; box-sizing:border-box;',
            ),
        );

        foreach ($bundles as $bundle) {
          $bundle_label = function_exists('node_type_get_name') ? node_type_get_name($bundle) : $bundle;

          $opts = array('' => t('- Use global fallback -')) + $this->vforms_variant_options_for_bundle($bundle);

          $cur = isset($current_map[$bundle]) ? (string) $current_map[$bundle] : '';
          if ($cur !== '' && !isset($opts[$cur])) {
            $opts = array($cur => t('@key (missing)', array('@key' => $cur))) + $opts;
          }

          $form['vforms_settings']['vforms_variant_map'][$bundle] = array(
            '#type' => 'select',
            '#title' => $bundle_label,
            '#options' => $opts,
            '#default_value' => $cur,
            '#parents' => array('options', 'vforms_variant_map', $bundle),
          );
        }
      }
    }

    $form['vforms_settings']['vforms_modal'] = array(
      '#type' => 'checkbox',
      '#title' => t('Open in modal dialog'),
      '#default_value' => !empty($this->options['vforms_modal']) ? 1 : 0,
      '#parents' => array('options', 'vforms_modal'),
    );

    $form['vforms_settings']['vforms_dialog_width'] = array(
      '#type' => 'textfield',
      '#title' => t('Dialog width'),
      '#default_value' => (string) $this->options['vforms_dialog_width'],
      '#size' => 6,
      '#description' => t('Pixels. Used only when modal is enabled.'),
      '#element_validate' => array('vforms_validate_int_string'),
      '#parents' => array('options', 'vforms_dialog_width'),

      // Enhancement: only show when modal is checked.
      '#states' => array(
        'visible' => array(
          ':input[name="options[vforms_modal]"]' => array('checked' => TRUE),
        ),
      ),
    );

  }

  /**
   * Render the link.
   */
  function render($values) {
    $nid = 0;

    if (!empty($values->nid)) {
      $nid = (int) $values->nid;
    }
    elseif (!empty($values->_entity) && is_object($values->_entity) && !empty($values->_entity->nid)) {
      $nid = (int) $values->_entity->nid;
    }
    elseif (!empty($values->node) && is_object($values->node) && !empty($values->node->nid)) {
      $nid = (int) $values->node->nid;
    }

    if (!$nid) return '';

    $node = node_load($nid);
    if (!$node) return '';

    if (!node_access('update', $node)) return '';

    $variant = trim($this->vforms_scalar($this->options['vforms_variant'], ''));
    if ($variant === '') {
      $variant = 'default';
    }

    // Per-type override.
    $map = (!empty($this->options['vforms_variant_map']) && is_array($this->options['vforms_variant_map']))
      ? $this->options['vforms_variant_map']
      : array();

    if (!empty($node->type) && isset($map[$node->type])) {
      $v = trim($this->vforms_scalar($map[$node->type], ''));
      if ($v !== '') {
        $variant = $v;
      }
    }

    // Basic sanity.
    if (!preg_match('/^[a-z0-9_]+$/', $variant)) {
      $variant = 'default';
    }

    $path = "node/{$nid}/form/{$variant}";

    $raw_text = !empty($this->options['text']) ? $this->options['text'] : t('%');
    $raw_text = $this->vforms_scalar($raw_text, t('%'));

    $options = array();

    // Match core edit-link behavior.
    $options['query'] = array('destination' => current_path());

    // Expand % into icon markup (and allow escaped %).
    if (strpos($raw_text, '%') !== FALSE) {
      $text = $this->vforms_expand_display_text_with_icon($raw_text, 'note-pencil');
      $options['html'] = TRUE;
    }
    else {
      $text = $raw_text;
    }

    if (!empty($this->options['vforms_modal'])) {
      // Ensure AJAX + dialog behaviors exist on the *page containing the link*.
      backdrop_add_library('system', 'backdrop.ajax');
      @backdrop_add_library('system', 'ui.dialog');

      // Ensure your target div is present.
      backdrop_add_library('vforms', 'dialog_target');

      // Dialog helpers (includes Ajax command definitions used after save).
      backdrop_add_library('vforms', 'dialog');

      $path .= '/dialog';
      $width = trim((string) $this->options['vforms_dialog_width']);
      if ($width === '' || !preg_match('/^\d+$/', $width)) {
        $width = 400;
      }

      $options['attributes']['class'][] = 'use-ajax vforms-edit-icon';
      $options['attributes']['data-dialog'] = 'true';
      $options['attributes']['data-dialog-type'] = 'dialog';
      $options['attributes']['data-dialog-options'] = json_encode(array(
        'width'     => (int) $width,
        'target'    => '#vforms-dialog-target',
        'modal'     => FALSE, // for jQuery UI modal overlay
        'draggable' => TRUE,
        'resizable' => TRUE,
      ));
    }

    return l($text, $path, $options);
  }
}
