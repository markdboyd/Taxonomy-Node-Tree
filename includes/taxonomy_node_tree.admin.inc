<?php

/**
 * @file
 * Provides infrequently used functions for taxonomy_node_tree
 */
/**
 * Taxonoy Node Tree admin settings form.
 *
 * @return
 *   The settings form used by Taxonomy Node Tree
 */
function taxonomy_node_tree_admin_settings_build_form() {

  $form['use_accordion'] = array(
	'#type' => 'checkbox',
	'#title' => t('Use Accordion Effect for Taxonomy Node Tree'),
	'#default_value' => variable_get('use_accordion', 1),
	'#description' => t("Enabling this option will provide an accordion like effect to navigate the taxonomy node tree.")	
  );	

  $form['use_taxonomy_node_tree_css'] = array(
	'#type' => 'checkbox',
	'#title' => t('Use default CSS'),
	'#default_value' => variable_get('use_taxonomy_node_tree_css', 1),
	'#description' => t("Use the default CSS for the taxonomy node tree.")	
  );	

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Menu callback for admin settings
 * @return array
 */
function taxonomy_node_tree_admin_settings() {

	$form = taxonomy_node_tree_admin_settings_build_form();
	
	$form = system_settings_form($form);
	$form['#submit'][] = 'taxonomy_node_tree_admin_settings_submit';

	return $form;
}

/**
 * Menu callback: display the taxonomy node tree block addition form.
 */
function taxonomy_node_tree_add_block_form(&$form_state) {
  module_load_include('inc', 'block', 'block.admin');
  return block_admin_configure($form_state, 'taxonomy_node_tree', NULL);
}

/**
 * Save the new taxonomy node tree
 */
function taxonomy_node_tree_add_block_form_submit($form, &$form_state) {
	
  // Determine the delta of the new block.
  $block_ids = variable_get('taxonomy_node_tree_block_ids', array());
  $delta = empty($block_ids) ? 1 : max($block_ids) + 1;

  // Save the new array of blocks IDs.
  $block_ids[] = $delta;
  variable_set('taxonomy_node_tree_block_ids', $block_ids);

  // Save the block configuration.
  _taxonomy_node_tree_block_save($delta, $form_state['values']);

  // Run the normal new block submission (borrowed from block_add_block_form_submit).
  foreach (list_themes() as $key => $theme) {
    if ($theme->status) {
      db_query("INSERT INTO {blocks} (visibility, pages, custom, title, module, theme, status, weight, delta, cache) VALUES(%d, '%s', %d, '%s', '%s', '%s', %d, %d, %d, %d)", $form_state['values']['visibility'], trim($form_state['values']['pages']), $form_state['values']['custom'], $form_state['values']['title'], $form_state['values']['module'], $theme->name, 0, 0, $delta, BLOCK_NO_CACHE);
    }
  }

  foreach (array_filter($form_state['values']['roles']) as $rid) {
    db_query("INSERT INTO {blocks_roles} (rid, module, delta) VALUES (%d, '%s', '%s')", $rid, $form_state['values']['module'], $delta);
  }

  drupal_set_message(t('The block has been created.'));
  cache_clear_all();

  $form_state['redirect'] = 'admin/build/block';
  return;
}

/**
 * Alters the block admin form to add delete links next to taxonomy node trees.
 */
function _taxonomy_node_tree_form_block_admin_display_form_alter(&$form, $form_state) {
  foreach (variable_get('taxonomy_node_tree_block_ids', array()) AS $delta) {
    $form['taxonomy_node_tree_' . $delta]['delete'] = array('#value' => l(t('delete'), 'admin/build/block/delete-taxonomy-node-tree-block/'. $delta));
  }
}

/**
 * Menu callback: confirm deletion of taxonomy node trees.
 */
function taxonomy_node_tree_delete(&$form_state, $delta = 0) {
  $config = taxonomy_node_tree_get_config($delta);
  $title = $config['admin_title'];
  $form['block_title'] = array('#type' => 'hidden', '#value' => $title);
  $form['delta'] = array('#type' => 'hidden', '#value' => $delta);

  return confirm_form($form, t('Are you sure you want to delete the "%name" block?', array('%name' => $title)), 'admin/build/block', NULL, t('Delete'), t('Cancel'));
}

/**
 * Deletion of taxonomy node trees.
 */
function taxonomy_node_tree_delete_submit($form, &$form_state) {
  // Remove the taxonomy node tree configuration variables.
  $delta = $form_state['values']['delta'];
  $block_ids = variable_get('taxonomy_node_tree_block_ids', array());
  unset($block_ids[array_search($delta, $block_ids)]);
  sort($block_ids);
  
  variable_set('taxonomy_node_tree_block_ids', $block_ids);
  variable_del("taxonomy_node_tree_{$delta}_admin_title");
  variable_del("taxonomy_node_tree_{$delta}_parent");
  variable_del("taxonomy_node_tree_{$delta}_max_depth");  
  variable_del("taxonomy_node_tree_{$delta}_node_visibility");
  variable_del("taxonomy_node_tree_{$delta}_term_filters");
  variable_del("taxonomy_node_tree_{$delta}_content_filters");
  variable_del("taxonomy_node_tree_{$delta}_content_filter_method");  
  variable_del("taxonomy_node_tree_{$delta}_collapse_inactive");
  
  db_query("DELETE FROM {blocks} WHERE module = 'taxonomy_node_tree' AND delta = %d", $delta);
  db_query("DELETE FROM {blocks_roles} WHERE module = 'taxonomy_node_tree' AND delta = %d", $delta);
  drupal_set_message(t('The "%name" block has been removed.', array('%name' => $form_state['values']['block_title'])));
  cache_clear_all();
  $form_state['redirect'] = 'admin/build/block';
  return;
}

/**
 * Returns the 'list' $op info for hook_block().
 */
function _taxonomy_node_tree_block_list() {
  $blocks = array();
  foreach (variable_get('taxonomy_node_tree_block_ids', array()) AS $delta) {
	$config = taxonomy_node_tree_get_config($delta);
	$title = $config['admin_title'];
    $blocks[$delta]['info'] = $title;
    // taxonomy node trees can't be cached because each menu item can have
    // a custom access callback. menu.inc manages its own caching.
    $blocks[$delta]['cache'] = BLOCK_NO_CACHE;
  }
  return $blocks;
}

/**
 * Returns the 'configure' $op info for hook_block().
 */
function _taxonomy_node_tree_block_configure($delta) {
  // Create a pseudo form state.
  $form_state = array('values' => taxonomy_node_tree_get_config($delta));
  return taxonomy_node_tree_configure_form($form_state);
}

/**
 * Returns the configuration form for a menu tree.
 *
 * @param $form_state
 *   array An associated array of configuration options should be present in the
 *   'values' key. If none are given, default configuration is assumed.
 * @return
 *   array The form in Form API format.
 */
function taxonomy_node_tree_configure_form(&$form_state) {				
	$config = array();

	// Merge in the default configuration.
	$config += taxonomy_node_tree_get_config();

	// Merge in existing settings
	$config = array_merge($config, $form_state['values']);

	// Get all term options
	$vocabs = taxonomy_get_vocabularies();

	foreach($vocabs as $vocab){	
		$term_tree = taxonomy_node_tree_build($vocab->vid);		
		$term_options[$vocab->name] = taxonomy_node_tree_build_term_options($vocab->vid, $term_tree);
		$exclude_options[$vocab->name] = taxonomy_node_tree_build_term_options($vocab->vid, $term_tree, false);		
	}
    
	// Get all content type options
	$content_options = array();

	$content_types = db_query('SELECT type, name from node_type');
	while($content_result = db_fetch_array($content_types)){
		$content_options[$content_result['type']] = $content_result['name'];
	}

	// Build the standard form.
	$form['taxonomy-node-tree-block-wrapper-start'] = array(
		'#value' => '<div id="taxonomy-node-tree-block-settings" class="taxonomy-node-tree-block-configure-form">'
	);

	$form['admin_title'] = array(
		'#type' => 'textfield',
		'#default_value' => $config['admin_title'],
		'#title' => t('Administrative title'),
		'#description' => t('This title will be used administratively to identify this block. If blank, the regular title will be used.'),
	);
	
	$form['tree_context'] = array(
      '#type' => 'fieldset',
      '#title' => t('Tree Settings'),     
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#description' => t('Set the root of your tree and how many levels deep it should go in the taxonomy structure.'),
    );

	$form['tree_context']['parent'] = array(
		'#type' => 'select',
		'#title' => t('Parent'),
		'#default_value' => $config['parent'],
		'#options' => $term_options,
		'#description' => t('Define where this menu should start, either at the root of the taxonomy tree or below any of its children'),
		'#attributes' => array('class' => 'taxonomy-node-tree-parent'),
	);

	$form['tree_context']['max_depth'] = array(
		'#type' => 'select',
		'#title' => t('Maximum depth'),
		'#default_value' => $config['max_depth'],
		'#options' => array(
		  '1'  => '1',
		  '2'  => '2',
		  '3'  => '3',
		  '4'  => '4',
		  '5'  => '5',
		  '6'  => '6',
		  '7'  => '7',
		  '8'  => '8',
		  '9'  => '9',
		  '0'  => t('Unlimited'),
		),
		'#description' => t('The value you choose here determines how many levels below your starting point the taxonomy tree block will show.'),
	);

	$form['tree_context']['collapse_inactive'] = array(
		'#type' => 'checkbox',
		'#title' => t('Collapse inactive portions of the tree?'),
		'#default_value' => $config['collapse_inactive'],
	);   

    $form['node_visibility'] = array(
      '#type' => 'fieldset',
      '#title' => t('Node Settings'),     
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t('Select whether or not to include nodes in your tree. Obviously, without nodes, the tree is just of taxonomy terms.'),
    );
    
	$form['node_visibility']['include_nodes'] = array(
	  '#type' => 'checkbox',
	  '#title' => t('Include nodes in this taxonomy tree'),
 	  '#default_value' => $config['node_visibility'],
	  '#description' => t('If you check this box, a list of nodes for each taxonomy term will be included in the output, otherwise the node will be ignored and the tree will just contain taxonomy terms.'),
	);	

    $form['term_visibility'] = array(
      '#type' => 'fieldset',
      '#title' => t('Term Settings'),     
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t('Choose what terms, if any, to exclude from your taxonomy node tree.'),
    );

	$form['term_visibility']['term_filters'] = array(
		'#type' => 'select',
		'#title' => t('Terms to Exclude'),
		'#default_value' => $config['term_filters'],
		'#options' => $exclude_options,	
		'#multiple' => true,
		'#description' => t('Choose any terms want to exclude from your tree'),
	);

    $form['content_visibility'] = array(
      '#type' => 'fieldset',
      '#title' => t('Content Type Settings'),     
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t('Choose what content types you want to include or exclude as node children in your tree.'),
    );
    
	$form['content_visibility']['content_filters'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Content Types to Include/Exclude'),
		'#default_value' => $config['content_filters'],
		'#options' => $content_options,	
		'#multiple' => true,	
	);
	
	$form['content_visibility']['content_filter_method'] = array(
		'#type' => 'radios',		
		'#title' => t('How to apply these content type filters'),			
		'#default_value' => $config['content_filter_method'],
		'#options' => array(
		    'include' => t('Include only these content types'), 
		    'exclude' => t('Exclude only these content types')
		),			
	);

	$form['taxonomy-node-tree-block-wrapper-close'] = array('#value' => '</div>');

	return $form;

}

function taxonomy_node_tree_build_term_options ( $vid, $tree , $include_vid = true, $depth = 0) {		
	$term_options = array();
		
	if($include_vid){
		$term_options[$vid . '-0'] = 'Root of the tree';
	}

	$i = 0;
	$name_prefix = '';
	while($i <= $depth){
		$name_prefix .= '--';
		$i++;
	}
	
	foreach($tree as $term){
		$tid = $term->tid;
		
		if($include_vid){
			$starting_level = $vid . '-' . $tid;		
		}
		else{
			$starting_level = $tid;				
		}
	
		$term_options[$starting_level] = $name_prefix . $term->name;

		if($term->children){			
			$term_options_more = array();
			$term_options_more = taxonomy_node_tree_build_term_options($vid, $term->children, $include_vid, $depth + 1);			
			if(is_array($term_options_more)){
				$term_options = $term_options + $term_options_more;
			}
		}
	}

	return $term_options;
}

/**
 * Returns the 'save' $op info for hook_block().
 */
function _taxonomy_node_tree_block_save($delta, $edit) {
  variable_set("taxonomy_node_tree_{$delta}_admin_title", $edit['admin_title']);
  variable_set("taxonomy_node_tree_{$delta}_parent", $edit['parent']);
  variable_set("taxonomy_node_tree_{$delta}_max_depth", $edit['max_depth']);  
  variable_set("taxonomy_node_tree_{$delta}_node_visibility", $edit['include_nodes']);
  variable_set("taxonomy_node_tree_{$delta}_term_filters", $edit['term_filters']);
  variable_set("taxonomy_node_tree_{$delta}_content_filters", $edit['content_filters']);
  variable_set("taxonomy_node_tree_{$delta}_content_filter_method", $edit['content_filter_method']);  
  variable_set("taxonomy_node_tree_{$delta}_collapse_inactive", $edit['collapse_inactive']);
}