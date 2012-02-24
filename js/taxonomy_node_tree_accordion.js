//Provides an accordion like effect for the taxonomy node tree menu

jQuery(function(){
	$active_ul = jQuery('.tree-root li.active-trail').children('ul');
	jQuery('.tree-root ul').not($active_ul).hide();
	jQuery('.tree-root li.active-trail').addClass('expanded-trail');

	jQuery('.term-tree > li > a').click(function(event){

		$sibling_uls = jQuery(this).siblings('ul');
		$parent_uls = jQuery(this).parents('.term-tree');

		$parent_children = jQuery($parent_uls).siblings('.node-tree');

		jQuery('.term-tree li ul').not($sibling_uls).not($parent_uls).not($parent_children).each(function(){
			jQuery(this).slideUp(200);
		});

		if($parent_uls.length > 1){
			jQuery('.term-tree li').not('.term-tree li', $parent_uls).removeClass('expanded-trail');
			jQuery(this).parent('.has-children').siblings('.has-children').removeClass('expanded-trail');
			jQuery('li', $sibling_uls).removeClass('expanded-trail');
		}
		else{
			jQuery('.term-tree li').removeClass('expanded-trail');
		}

		jQuery('.term-tree li a').removeClass('expanded');
		
		if($sibling_uls.length > 0){						
			jQuery($sibling_uls).each(function(){
				jQuery(this).slideDown(200);		
			});
			jQuery(this).parents('li').addClass('expanded-trail');		
			jQuery(this).addClass('expanded');
		}

		event.preventDefault();		
		return false;
	}); 
});

