<?php

	function taxonomy_node_tree_find_associated_content_types( $vid ){
		$query = db_query('SELECT type FROM vocabulary_node_types WHERE vid = %d', $vid);

		while($types_result = db_fetch_array($query)){
			$associated_types[] = $types_result['type'];
		}
	
		return $associated_types;
	}

	function taxonomy_node_tree_render_vocab_associated_types_add_links ( $vid ) {
		$associated_types = taxonomy_node_tree_find_associated_content_types( $vid );

		$html = '';

		foreach($associated_types as $type){
			$name = end(vitalvoices_convert_get_content_name_from_type( $type ));
			$html .= context_admin_links_render_node_create_options( 
				array(
				'node_type' => $type,
				'caption' => 'Add ' . $name
				) 
			);
		}

		return $html;
	}
