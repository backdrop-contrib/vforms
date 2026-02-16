<?php
/**
 * @file
 * Views integration for Node Form Variants.
 * commit concurrency: 401
 */


/**
 * Implements hook_views_data_alter().
 * 
 */
function vforms_views_data_alter(&$data) {
  // Add a new field to the node table similar to the core "Edit link" field,
  // but allowing selection of a form variant and optional modal dialog.
  $data['node']['vforms_edit_link'] = array(
    'title' => t('Form variant edit link'),
    'help' => t('Link to edit the node using a selected form variant (optionally in a modal dialog).'),
    'real field' => 'nid',
    'click sortable' => FALSE,
    'field' => array(
      'handler' => 'vforms_views_handler_field_node_link_vforms_edit',
      'file' => 'views/vforms.views_handler_field_node_link_vforms_edit.inc',
    ),
  );
}
