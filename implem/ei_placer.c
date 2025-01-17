/**
 *  @file	ei_placer.h
 *  @brief	Manages the positioning and sizing of widgets on the screen.
 *
 *  \author
 *  Created by François Bérard on 18.12.11.
 *  Copyright 2011 Ensimag. All rights reserved.
 *
 */



#include "ei_placer.h"
#include "ei_types.h"
#include "ei_widget.h"
#include "ei_implementation.h"
#include "ei_geometrymanager.h"
#include "ei_widget_configure.h"
#include "var.h"

/**
 * \brief	Configures the geometry of a widget using the "placer" geometry manager.
 *
 *		The placer computes a widget's geometry relative to its parent *content_rect*.
 *
 * 		If the widget was already managed by the "placer", then this calls simply updates
 *		the placer parameters: arguments that are not NULL replace previous values.
 *		Arguments that are NULL don't change the current value of the parameter.
 *
 * 		When the arguments are passed as NULL, the placer uses default values (detailed in
 *		the argument descriptions below). If no size is provided (either absolute or
 *		relative), then either the requested size of the widget is used if one was provided,
 *		or the default size is used.
 *
 * @param	widget		The widget to place.
 * @param	anchor		How to anchor the widget to the position defined by the placer
 *				(defaults to ei_anc_northwest).
 * @param	x		The absolute x position of the widget (defaults to 0).
 * @param	y		The absolute y position of the widget (defaults to 0).
 * @param	width		The absolute width for the widget (defaults to the requested width or
 * 				the default width of the widget if rel_width is NULL, or 0 otherwise).
 * @param	height		The absolute height for the widget (defaults to the requested height or
 *				the default height of the widget if rel_height is NULL, or 0 otherwise).
 * @param	rel_x		The relative x position of the widget: 0.0 corresponds to the left
 *				side of the parent, 1.0 to the right side (defaults to 0.0).
 * @param	rel_y		The relative y position of the widget: 0.0 corresponds to the top
 *				side of the parent, 1.0 to the bottom side (defaults to 0.0).
 * @param	rel_width	The relative width of the widget: 0.0 corresponds to a width of 0,
 *				1.0 to the width of the parent (defaults to 0.0).
 * @param	rel_height	The relative height of the widget: 0.0 corresponds to a height of 0,
 *				1.0 to the height of the parent (defaults to 0.0).
 */
void		ei_place	(ei_widget_t		widget,
				 ei_anchor_t*		anchor,
				 int*			x,
				 int*			y,
				 int*			width,
				 int*			height,
				 float*			rel_x,
				 float*			rel_y,
				 float*			rel_width,
				 float*			rel_height){

        /**
         * Ce qu'on veut de cette fonction : Qu'elle mette a jour correctement ces champs la de notre widget:
         * ei_size_t		requested_size;
         * ei_rect_t		screen_location;
	 * ei_rect_t*		content_rect;
         *
         * C'est au programmeur d'appeler cette fonction.
         * Place le widget:  - relativement a ses parents :widget->parent (avec les parametre rel)
         *                   - et avec les valeurs absolue (x, y, etc) (toujours relativement au parent)
         *
         */
        if (!widget) return;
        if (rel_height && *rel_height!=0){
                widget->requested_size.height = (widget->parent->requested_size.height) * (*rel_height);
                widget->screen_location.size.height = widget->requested_size.height;
        }
        if (rel_width && *rel_width!=0){
                widget->requested_size.width = (widget->parent->requested_size.width) * (*rel_width);
                widget->screen_location.size.width = widget->requested_size.width;
        }
        //widget->geom_params->manager= ei_geometrymanager_from_name((char*){"placer\0"});
        if (width && *width!=0){
                widget->screen_location.size.width = *width;
                widget->requested_size.width = *width;
        }
        if (height && *height!=0){
                widget->requested_size.height= *height;
                widget->screen_location.size.height = *height;
        }

        //calculer x et y en fonction de l'ancrage :

        int xpos=widget->parent->screen_location.top_left.x,ypos = widget->parent->screen_location.top_left.y;
        if (rel_x)
                xpos = xpos + (widget->parent->requested_size.width) * (*rel_x);
        if (rel_y)
                ypos = ypos + (widget->parent->requested_size.height) * (*rel_y);
        if (x) xpos = xpos + (*x);
        if (y) ypos = ypos + (*y);
        //printf("parent : %d %d , enfant : %d %d\n",widget->parent->screen_location.top_left.x, widget->parent->screen_location.top_left.y, xpos, ypos);
        ei_anchor_t anc;
        if (!anchor) anc= ei_anc_northwest;
        else anc = *anchor;

        switch (anc){
                case ei_anc_none:
                case ei_anc_northwest:
                        xpos = xpos;
                        ypos = ypos;
                        break;
                case ei_anc_north:
                        xpos = xpos-(widget->requested_size.width)/2;
                        ypos = ypos;
                        break;
                case ei_anc_northeast:
                        xpos = xpos - widget->requested_size.width;
                        ypos = ypos;
                        break;
                case ei_anc_west:
                        xpos = xpos;
                        ypos = ypos - widget->requested_size.height /2;
                        break;
                case ei_anc_center:
                        xpos = xpos-(widget->requested_size.width)/2;
                        ypos = ypos - widget->requested_size.height /2;
                        break;
                case ei_anc_east:
                        xpos = xpos - widget->requested_size.width;
                        ypos = ypos - widget->requested_size.height /2;
                        break;
                case ei_anc_southwest:
                        xpos = xpos;
                        ypos = ypos - widget->requested_size.height;
                        break;
                case ei_anc_south:
                        xpos = xpos - widget->requested_size.width / 2;
                        ypos = ypos - widget->requested_size.height;
                        break;
                case ei_anc_southeast:
                        xpos = xpos - widget->requested_size.width;
                        ypos = ypos - widget->requested_size.height;
                        break;
        }

        if (xpos!= 0) widget->screen_location.top_left.x = xpos;
        if (ypos!= 0) widget->screen_location.top_left.y = ypos;

        ei_placer_t *geom_param =NULL;
        if (widget->geom_params==NULL){
                geom_param = calloc(1,sizeof(ei_placer_t));
                ei_widget_set_geom_params(widget,(ei_geom_param_t)geom_param);
                ei_widget_set_geom_manager(widget,ei_geometrymanager_from_name("placer\0"));
        }else
        {
                 geom_param = (ei_placer_t*)(widget->geom_params);
        }

        //*x,*y,*height,*width,*rel_x,*rel_y,*rel_height,*rel_width,*anchor};
        if (x) geom_param->x           = *x;
        if (y) geom_param->y           = *y;
        if (height) geom_param->height      = *height;
        if (width) geom_param->width       = *width;
        if (rel_x) geom_param->rel_x       = *rel_x;
        if (rel_y) geom_param->rel_y       = *rel_y;
        if (rel_height) geom_param->rel_height  = *rel_height;
        if (rel_width) geom_param->rel_width   = *rel_width;
        if (anchor) geom_param->anchor      = *anchor;

        ei_rect_t rect = get_rect_intersection(widget->parent->screen_location,ei_app_root_widget()->screen_location);

        ei_app_invalidate_rect(&rect);
        ei_widget_t child = widget->children_head;
        while (child){
                if ((child->geom_params) && (child->geom_params->manager)) (child->geom_params->manager->runfunc)(child);
                child=child->next_sibling;
        }

}

