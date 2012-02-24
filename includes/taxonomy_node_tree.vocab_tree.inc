<?php

/* Builds a tree of vocabularies */
function taxonomy_node_tree_build_vocab_tree ( $vid, $include = array() ){
	$vocabs = taxonomy_get_vocabularies();

	$vocab_tree = array();

	foreach($vocabs as $vocab){
		if(isset($include) && count($include)){
			if(in_array($vocab->vid, $include)){
				$vocab_tree[$vocab->vid] = $vocab;	

				if($vid == $vocab->vid){
					$vocab_tree[$vocab->vid]->in_vocabulary_active_trail = true;	
				}			
			}	
			else{
				continue;
			}
		}	
		else{
			$vocab_tree[$vocab->vid] = $vocab;

			if($vid == $vocab->vid){
				$vocab_tree[$vocab->vid]->in_vocabulary_active_trail = true;	
			}			
		}			
	}

	return $vocab_tree;
}

function taxonomy_node_tree_vocab_tree_output ( $vid, $include = array() ){
	$vocab_tree = taxonomy_node_tree_build_vocab_tree($vid, $include);

	$html = '<ul class="vocab-tree">';
	
	foreach($vocab_tree as $vocab){
		$class = '';

		$vocab_info = taxonomy_node_tree_get_vocab_path($vocab->vid);

		$vocab_path = $vocab_info['path'];
		$vocab_name = $vocab_info['name'];
		
		if($vocab->in_vocabulary_active_trail){
			$class = ' active-trail';
		}

		$html .= '<li class="vocab-item' . $class . '">';
		$html .= l($vocab_name, $vocab_path);
		$html .= '</li>';	
	}

	$html .= '</ul>';

	return $html;
}

function taxonomy_node_tree_get_vocab_path($vid){
	$results = array();

	$vocab = taxonomy_vocabulary_load($vid);
	
	$results['path'] = end(module_invoke_all('taxonomy_node_tree_get_vocab_path', $vid));
	$results['name'] = $vocab->name;

	return $results;
}