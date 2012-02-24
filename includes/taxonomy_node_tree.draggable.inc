<?php

	function taxonomy_node_tree_tid_has_sorted_nodes($tid, $content_filters){
		$has_sort = taxonomy_node_tree_build_sort_query($tid, $content_filters);
		$sorted_result = db_query($has_sort);
		$is_sorted = db_fetch_object($sorted_result);
	
		if($is_sorted && count($is_sorted)){
			return true;
		}

		return false;
	}

	function taxonomy_node_tree_build_sort_query($tid, $content_filters){
		if($content_filters['filters'] && count($content_filters['filters'])){
			$type_filter = 'AND node.type ';
			$filter_count = 1;
	
			switch($content_filters['method']){
				case 'exclude':
				    $type_filter .= 'NOT IN ('; 	
					break;

				case 'include':
					$type_filter .= 'IN ('; 
					break;
			}

			$filters = taxonomy_node_tree_get_applicable_filters($content_filters['filters'], $content_filters['method']);

			foreach($filters as $filter){
				if($filter_count != count($filters)){
					$extra = ', ';
				}
				if(is_string($filter)){
					$type_filter .= '"' . $filter .'"' . $extra;
				}
				$filter_count++;
			}

			$type_filter .= ') ';
		}

		$has_sort = 'SELECT node.nid ' . 
					'FROM term_node, draggableviews_structure, node ' . 
					'WHERE term_node.nid = draggableviews_structure.nid ' . 
					'AND node.nid = draggableviews_structure.nid ' . 
					'AND node.nid = term_node.nid ' . 
					$type_filter . 
					'AND draggableviews_structure.delta = 0 ' . 
					'AND term_node.tid = ' . $tid . ' ' . 
					'ORDER BY draggableviews_structure.value ASC';

		return $has_sort;
	}

	function taxonomy_node_tree_get_applicable_filters($content_filters, $method){
		$filters = array();

		foreach($content_filters as $filter){
			switch($method){
				case 'exclude':
					if(!is_string($filter)){
						$filters[] = $filter;
					}
					break;

				case 'include':
					if(is_string($filter)){
						$filters[] = $filter;
					}
					break;
			}
		}

		return $filters;
	}
