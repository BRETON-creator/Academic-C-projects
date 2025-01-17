/**
 *  @file	ei_geometrymanager.h
 *  @brief	Manages the positioning and sizing of widgets on the screen.
 *
 *  \author
 *  Created by François Bérard on 18.12.11.
 *  Copyright 2011 Ensimag. All rights reserved.
 *
 */


#include "ei_geometrymanager.h"
#include "ei_implementation.h"
#include "var.h"


/**
 * on initialise une variable globale contenant tous les geometrymanager
 */

ei_geometrymanager_t* head_list_geometrymanager = NULL;

/**
 * \brief	Returns the size of \ref ei_impl_geom_param_t.
 */
size_t		ei_geom_param_size(){
        return sizeof(ei_impl_geom_param_t);
}





/**
 * \brief	Must be called by runfuncs (see \ref ei_geometrymanager_runfunc_t).
 *
 * 		Checks if the geometry computation resulted in a change of geometry of
 * 		the widget (size and/or position). If there was a change:
 * 			* schedule a redraw of the screen on the old and new screen location
 * 			* notify the widget that it's geometry has changed
 * 			* recompute the geometry of the children
 *
 * @param	widget		The which geometry computation has been done.
 * @param	computed_screen_location The new screen location computed by the run of the
 * 				geometry manager. May be the same as the screen location of
 * 				the widget before the run.
 */
void			ei_geometry_run_finalize(ei_widget_t widget, ei_rect_t* new_screen_location){
        if (widget->screen_location.top_left.x != new_screen_location->top_left.x ||
            widget->screen_location.top_left.y != new_screen_location->top_left.y ||
            widget->screen_location.size.height!= new_screen_location->size.height||
            widget->screen_location.size.width != new_screen_location->size.width){
                //alors la geometrie a changée
                //on met a jour la screen location
                widget->screen_location = *new_screen_location;
                //on prevoie un redraw (si le widget est manager par un geometrymanager)
                //on notifie le widget qu'on a changé sa géométrie
                if (widget->wclass->geomnotifyfunc)(widget->wclass->geomnotifyfunc)(widget);
                //on recalcule la géométrie des enfants
                ei_widget_t child = widget->children_head;
                while (child){
                        if ((child->geom_params) && (child->geom_params->manager)) (child->geom_params->manager->runfunc)(child);
                        child=child->next_sibling;
                }

                widget->screen_location = *new_screen_location;
        }

}



/**
 * \brief	Registers a geometry manager to the program so that it can be called to manager
 *		widgets. This must be done only once in the application.
 *
 * @param	geometrymanager		The structure describing the geometry manager.
 */
void			ei_geometrymanager_register	(ei_geometrymanager_t* geometrymanager){
        ei_geometrymanager_t* tmp = head_list_geometrymanager;
        head_list_geometrymanager = geometrymanager;
        geometrymanager->next = tmp;
}



/**
 * \brief	Returns a geometry manager structure from its name.
 *
 * @param	name		The name of the geometry manager.
 *
 * @return			The structure describing the geometry manager.
 */
ei_geometrymanager_t*	ei_geometrymanager_from_name	(ei_geometrymanager_name_t name){
        ei_geometrymanager_t* current = head_list_geometrymanager;
        while (current){
                if (strcmp(current->name,name)==0) return current;
                current=current->next;
        }
        return NULL;
}



/**
 * \brief	Tell the geometry manager in charge of a widget to forget it. This removes the
 *		widget from the screen. If the widget is not currently managed, this function
 *		returns silently.
 *		Side effects:
 *		<ul>
 *			<li> the \ref ei_geometrymanager_releasefunc_t of the geometry manager in
 *				charge of this widget is called, </li>
 *			<li> the geom_param field of the widget is freed, </li>
 *			<li> the current screen_location of the widget is invalidated (which will
 *				trigger a redraw), </li>
 *			<li> the screen_location of the widget is reset to 0. </li>
 *		</ul>
 *
 * @param	widget		The widget to unmap from the screen.
 */
void			ei_geometrymanager_unmap	(ei_widget_t widget){
        if (!widget->geom_params || !widget->geom_params->manager)
                return;

        (widget->geom_params->manager->releasefunc)(widget);

        if ((widget->wclass->geomnotifyfunc))
                (widget->wclass->geomnotifyfunc)(widget);

        widget->screen_location = (ei_rect_t){{0,0},{0,0}};
        widget->geom_params=NULL;
}



/**
 * @brief	Get the geometry manager for this widget.
 *
 * @param	widget		The widget.
 *
 * @return			A pointer to the geometry manager, or NULL if the widget is not currently displayed.
 */
ei_geometrymanager_t*	ei_widget_get_geom_manager	(ei_widget_t widget){
        return widget->geom_params ? widget->geom_params->manager : NULL;
}

/**
 * @brief	Sets the geometry manager for this widget.
 *
 * @param	widget		The widget.
 * @param	manager		The geometry manager managing this widget.
 */
void			ei_widget_set_geom_manager	(ei_widget_t widget, ei_geometrymanager_t* manager){
        widget->geom_params->manager = manager;
}

/**
 * @brief	Get the geometry management parameters for this widget.
 *
 * @param	widget		The widget.
 *
 * @return			A pointer to the geometry management parameters, or NULL if the widget is not currently displayed.
 */
ei_geom_param_t		ei_widget_get_geom_params	(ei_widget_t widget){
        return widget->geom_params;
}

/**
 * @brief	Sets the geometry management parameters for this widget.
 *
 * @param	widget		The widget.
 * @param	geom_params	The geometry management parameters.
 */
void			ei_widget_set_geom_params	(ei_widget_t widget, ei_geom_param_t geom_param){
        widget->geom_params=geom_param;
}
