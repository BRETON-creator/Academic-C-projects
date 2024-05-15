//
// Created by montelr on 5/14/24.
//

#include "ei_implementation.h"
#include "ei_draw.h"
#include "ei_placer.h"
#include "ei_event.h"
#include "var.h"
#include "hw_interface.h"
#include "ei_types.h"
#include "ei_implementation.h"
#include "ei_widget_configure.h"



/**
* \brief Fonction pour obtenir l'union de deux rectangles
*
*/

//utile pour le redessin ?

ei_rect_t get_rect_union( ei_rect_t old_rect , ei_rect_t new_rect){
    ei_rect_t union_rect;
    //pour le top_left on choisit entre les deux top_left des rectangles celui qui est le plus proche de l'origine
    union_rect.top_left.x  = old_rect.top_left.x <= new_rect.top_left.x ? old_rect.top_left.x : new_rect.top_left.x;
    union_rect.top_left.y  = old_rect.top_left.y <= new_rect.top_left.y ? old_rect.top_left.y : new_rect.top_left.y;
    // On determine la size du rectangle englobant
    int max_x  = old_rect.top_left.x + old_rect.size.width <= new_rect.top_left.x + new_rect.size.width ?  new_rect.top_left.x + new_rect.size.width: old_rect.top_left.x + old_rect.size.width;
    int max_y  = old_rect.top_left.y + old_rect.size.height <= new_rect.top_left.y + new_rect.size.height ?  new_rect.top_left.y + new_rect.size.height: old_rect.top_left.y + old_rect.size.height;
    union_rect.size.height = max_x  -  union_rect.top_left.x;
    union_rect.size.width = max_y -  union_rect.top_left.y;

    return union_rect;

}

/**
* \brief Fonction pour obtenir l'intersection_rection de deux rectangles
*
*/

ei_rect_t get_rect_intersection( ei_rect_t old_rect , ei_rect_t new_rect){
    ei_rect_t intersection_rect;
    intersection_rect.top_left.x = (old_rect.top_left.x > new_rect.top_left.x) ? old_rect.top_left.x : new_rect.top_left.x;
    intersection_rect.top_left.y = (old_rect.top_left.y > new_rect.top_left.y) ? old_rect.top_left.y : new_rect.top_left.y;

    intersection_rect.size.width = (( old_rect.top_left.x + old_rect.size.width < new_rect.top_left.x + new_rect.size.width) ?  old_rect.top_left.x + old_rect.size.width : new_rect.top_left.x + new_rect.size.width) - intersection_rect.top_left.x;
    intersection_rect.size.height = ((old_rect.top_left.y + old_rect.size.height < new_rect.top_left.y + new_rect.size.height) ? old_rect.top_left.y + old_rect.size.height : new_rect.top_left.y + new_rect.size.height) - intersection_rect.top_left.y;

    if (intersection_rect.size.width < 0 || intersection_rect.size.height < 0) {
        intersection_rect.size.width = 0;
        intersection_rect.size.height = 0;
    }

    return intersection_rect;
}

//============================= toplevel

/**
 *  \brief fonction pour alloué un espace pour un widget toplevel.
 *
 * @return un espace assez grand pour stocker un widget de classe toplevel.
 */
ei_widget_t ei_impl_alloc_toplevel(){
        return calloc(1,sizeof(ei_impl_toplevel_t));
}

/**
 * \brief Fonction pour free un espace alloué a un widget toplevel.
 *
 */
void ei_impl_release_toplevel(ei_widget_t toplevel){
        ei_widget_t widget = ((ei_impl_toplevel_t*)(toplevel))->widget.parent;
        ei_widget_t child=widget->next_sibling;
        while (true){
                if (child==NULL || child->pick_id==toplevel->pick_id) break;
                child=child->next_sibling;
        }
        if (child) child->parent->next_sibling=toplevel->next_sibling;
        if (widget->children_head->pick_id==toplevel->pick_id){
                if (widget->next_sibling) widget->children_head=widget->next_sibling;
                else widget->children_head=NULL;
        }
        free((ei_impl_toplevel_t*)toplevel);
}

/**
* \brief Fonction pour mettre les valeurs par defauts d'un widget toplevel
*/
bool toplevel_close(ei_widget_t	widget,
                    ei_event_t*	event,
                    ei_user_param_t user_param){
        widget->parent->geom_params->manager = NULL;
        ei_impl_widget_draw_children(ei_app_root_widget(),ei_app_root_surface(),pick_surface,&widget->parent->screen_location);
        ei_impl_release_button(widget);
        ei_impl_release_toplevel(widget->parent);

        return true;
}

void ei_impl_setdefaults_toplevel(ei_widget_t widget){
        ei_impl_toplevel_t* toplevel = (ei_impl_toplevel_t*)widget;

        toplevel->widget.wclass =ei_widgetclass_from_name((ei_const_string_t){"toplevel\0"});
        toplevel->widget.user_data = NULL;
        toplevel->widget.destructor = NULL;
        /* Widget Hierachy Management */
        toplevel->widget.parent = ei_app_root_widget();		///< Pointer to the parent of this widget.
        toplevel->widget.children_head=NULL;	///< Pointer to the first child of this widget.	Children are chained with the "next_sibling" field.
        toplevel->widget.children_tail=NULL;	///< Pointer to the last child of this widget.
        toplevel->widget.next_sibling=NULL;	///< Pointer to the next child of this widget's parent widget.

        /* Geometry Management */
        toplevel->widget.geom_params = (ei_geom_param_t){NULL};	///< Pointer to the geometry management parameters for this widget. If NULL, the widget is not currently managed and thus, is not displayed on the screen.
        toplevel->widget.requested_size=(ei_size_t){320,240} ;	///< See \ref ei_widget_get_requested_size.
        toplevel->widget.screen_location=(ei_rect_t){(ei_point_t){0,0},(ei_size_t){320,240}};///< See \ref ei_widget_get_screen_location.

        toplevel->color = &ei_default_background_color;
        int* border = calloc(1, sizeof(int));
        *border = 4;
        toplevel->border_width = border;
        toplevel->title="Toplevel";
        toplevel->can_close = true;
        toplevel->resizable_axis = ei_axis_both;
        toplevel->minimal_size = (ei_size_t){160, 120};



        ei_widget_t button = ei_widget_create	("button", toplevel, NULL, NULL);
        ei_button_configure		(button, &(ei_size_t){12, 12},
                                            &(ei_color_t){235, 20, 20, 255},
                                            &(int){1}, &(int){5},
                                            &(ei_relief_t){ei_relief_raised},
                                            NULL, NULL,
                                            NULL, NULL, NULL, NULL, NULL,
                                            &(ei_callback_t){toplevel_close}, NULL);



        ei_place(button, &(ei_anchor_t){ei_anc_northwest}, &(int){*border + 4}, &(int){*border} + 4, NULL,
                 NULL, &(float){0.0}, &(float){0.0}, NULL, NULL);
        toplevel->button = button;

        ei_color_t color  = *toplevel->color;
        ei_color_t dark_color  = (ei_color_t){color.red -50, color.green -50, color.blue -50, color.alpha};

        ei_widget_t resize_button = ei_widget_create	("button", toplevel, NULL, NULL);
        ei_button_configure		(resize_button, &(ei_size_t){10, 10},
                                            &dark_color,
                                            &(int){0}, &(float ){0},
                                            &(ei_relief_t){ei_relief_raised},
                                            NULL, NULL,
                                            NULL, NULL, NULL, NULL, NULL,
                                            &(ei_callback_t){NULL}, NULL);

        ei_place(resize_button, &(ei_anchor_t){ei_anc_southeast}, NULL, NULL, NULL, NULL, &(float){1.0}, &(float){1.0}, NULL, NULL);
        toplevel->resize_button = resize_button;
}

/**
* \brief Fonction pour dessiner un widget toplevel.
*
*/
void ei_impl_draw_toplevel(ei_widget_t widget, ei_surface_t surface, ei_surface_t pick_surface, ei_rect_t* clipper){

        hw_surface_unlock(surface);
        ei_impl_toplevel_t* toplevel = (ei_impl_toplevel_t*)widget;
        ei_rect_t new_clipper = get_rect_intersection(*clipper, widget->screen_location);
        ei_rect_t rect= widget->screen_location;

        ei_color_t color  = *toplevel->color;
        ei_color_t dark_color  = (ei_color_t){color.red -50, color.green -50, color.blue -50, color.alpha};

        int border = *toplevel->border_width;

        int radius = k_default_button_corner_radius;
        ei_point_t rounded_frame_temp[40];
        ei_point_t rounded_frame[21];

        give_rounded_frame(rounded_frame_temp, rect, radius);

        for (int i=0; i<21;i++){
                rounded_frame[i] = rounded_frame_temp[i];
        }

        ei_point_t square_frame[4] = {{rect.top_left.x , rect.top_left.y + radius},
                                      {rect.top_left.x + rect.size.width , rect.top_left.y + radius},
                                      {rect.top_left.x + rect.size.width , rect.top_left.y + rect.size.height },
                                      {rect.top_left.x, rect.top_left.y + rect.size.height}};

        ei_point_t little_square_frame[4] = {{rect.top_left.x + border , rect.top_left.y + 2*radius + border},
                                             {rect.top_left.x + rect.size.width - border, rect.top_left.y + 2*radius + border},
                                             {rect.top_left.x + rect.size.width - border, rect.top_left.y + rect.size.height - border},
                                             {rect.top_left.x + border, rect.top_left.y + rect.size.height - border}};


        ei_draw_polygon(surface,square_frame,4, dark_color, &new_clipper);
        ei_draw_polygon(surface,rounded_frame_temp,21, dark_color, &new_clipper);
        ei_draw_polygon(surface,little_square_frame,4, color, &new_clipper);
        ei_draw_polygon(pick_surface,rounded_frame,40,*(widget->pick_color),&new_clipper);


        ei_color_t white_color = (ei_color_t){255,255,255, 255};

        if (toplevel->title) {

                ei_point_t where = (ei_point_t){ border + rect.top_left.x + 2*toplevel->button->requested_size.width, rect.top_left.y + border};
                ei_font_t font = hw_text_font_create(ei_default_font_filename, ei_style_normal, 18);
                ei_draw_text(surface, &where, toplevel->title,
                             font, white_color,
                             clipper);
                hw_text_font_free(font);
        }

        ei_app_invalidate_rect(&widget->screen_location);

        hw_surface_lock(surface);
}
bool toplevel_move;
ei_point_t mouse_point;

void move_child(ei_widget_t widget, int x, int y){
        widget->screen_location.top_left.x += x;
        widget->screen_location.top_left.y += y;

        ei_widget_t child = widget->children_head;
        while (child){
                move_child(child, x, y);
                child = child->next_sibling;
        }
}

bool ei_callback_toplevel(ei_widget_t		widget, struct ei_event_t*	event, ei_user_param_t	user_param){

        if (event->type==ei_ev_mouse_buttonup){
                toplevel_move = 0;
                return 1;
        }
        if (!widget) return false;

        ei_point_t cur_point = event->param.mouse.where;
        ei_impl_toplevel_t* toplevel = (ei_impl_toplevel_t*)widget;
        ei_rect_t rect = toplevel->widget.screen_location;


        if (toplevel_move==0 && event->type==ei_ev_mouse_buttondown){
                if (rect.top_left.y<=cur_point.y && cur_point.y<=rect.top_left.y+k_default_button_corner_radius*2+*toplevel->border_width){
                        toplevel_move=1;
                        mouse_point = event->param.mouse.where;
                        return 1;
                }

        }

        if (toplevel_move && event->type==ei_ev_mouse_move){
                int x = cur_point.x - mouse_point.x;
                int y = cur_point.y - mouse_point.y;

                move_child(widget, x, y);
                mouse_point=cur_point;

                rect.size.width++;
                rect.size.height++;
                ei_impl_widget_draw_children(ei_app_root_widget(), ei_app_root_surface(), pick_surface, &rect);

                return 1;
        }
}




void ei_impl_geomnotify_toplevel(ei_widget_t widget){
    if (widget->geom_params) (widget->geom_params->manager->runfunc)(widget);
}