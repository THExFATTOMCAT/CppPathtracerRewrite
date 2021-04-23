#include "include.h"

	BoundingSphere bounding_sphere;
	Integrator * integrator;
	UnidirectionalIntegrator unidirectional_integrator;
	//LightIntegrator light_integrator;
	//MetropolisIntegrator metropolis_integrator;
    BidirectionalIntegrator_exp bidirectional_integrator_exp;
	unsigned int thread_count;
	
	BVH accelerator;
	Scene scene;
	
	char obj_path[128];
	scalar obj_scale;
	
	Light * light[16];
	
	Camera camera;
	scalar camera_pos[3];
	scalar camera_target[3];
	scalar camera_up[3];
	
	scalar focal_length;
	scalar pupil_size;
	scalar focus_distance;
	
	int focus_x_pixel;
	int focus_y_pixel;
	
	bool prepared;
	
	GtkWidget *entry_path;
	GtkWidget *entry_thread;
	GtkWidget *obj_fraction_scale;
	GtkWidget *entry_camera_depth;
	GtkWidget *entry_light_depth;
	GtkWidget *entry_samples;

	GtkWidget *sun_scale;
	GtkWidget *sun_bool;
	GtkWidget *sun_pos_x_scale;
	GtkWidget *sun_pos_y_scale;
	GtkWidget *sun_pos_z_scale;

	
	GtkWidget *cam_x_scale;
	GtkWidget *cam_y_scale;
	GtkWidget *cam_z_scale;
	
	GtkWidget *tar_x_scale;
	GtkWidget *tar_y_scale;
	GtkWidget *tar_z_scale;
	
	GtkWidget *bg_r_scale;
	GtkWidget *bg_g_scale;
	GtkWidget *bg_b_scale;
	
	GtkWidget *pupil_scale;
	GtkWidget *focus_scale;
	GtkWidget *focus_button;
	
	GtkWidget *entry_res_x;
	GtkWidget *entry_res_y;
	
	GtkWidget *focus_x_scale;
	GtkWidget *focus_y_scale;
	
	GtkWidget *smart_sample_button;
	GtkWidget *preview_button;
	GtkWidget *bidirectional_button;
	
	GtkWidget *window;
	GtkWidget *table;
	
	
	
	GtkWidget *material_editor;
	Triangle *selected_triangle;
	GtkWidget *material_label;
	GtkWidget *roughness_scale;
	GtkWidget *metallic_scale;
	GtkWidget *transmission_scale;
	GtkWidget *ior_scale;
	GtkWidget *material_color_button;
	GtkWidget *material_emission_color_button;
	GtkWidget *emission_scale;
	
	GtkWidget *list_box_container;
	GtkWidget *texture_list_box;
	
	GtkWidget *mask_button;
	
	GtkWidget *active_menu;
	
	const int max_texture_count = 128;
	Texture2D *texture_list[128];
	GtkWidget *texture_list_buttons[128];
	int texture_count;
	
	bool lock_material;
	
	ProceduralTexture2D * checker;
	ImageTexture * image_texture;


static void select_texture_func(GtkWidget *widget, gpointer data){
	int i = 0;
	if(selected_triangle == NULL){
		return;
	}
	while(i < texture_count){
		if(widget == texture_list_buttons[i]){
			selected_triangle->material->t_color = texture_list[i];
			return;
		}
		i ++;
	}
	
}

static void add_texture_to_list(Texture2D * tex, char * name){
	if(texture_count < max_texture_count){
		texture_list[texture_count] = tex;
		if(tex == NULL){
			texture_list_buttons[texture_count] = gtk_button_new_with_label("NULL");
		}
		else{
			texture_list_buttons[texture_count] = gtk_button_new_with_label((gchar *) name);
		}
		g_signal_connect(texture_list_buttons[texture_count], "clicked", G_CALLBACK(select_texture_func), NULL);
		gtk_list_box_insert ((GtkListBox*) texture_list_box, texture_list_buttons[texture_count], -1);
		
		
		
		texture_count ++;
	}
}



static void prepare_func (GtkWidget *widget,
             gpointer   data)
			 {
	selected_triangle = NULL;
	setlocale (LC_NUMERIC, "C");
    char buffer[256];
	
	strcpy(buffer, gtk_entry_get_text((GtkEntry*)entry_path));
	std::cout << buffer;
	
	std::cout << "thread_count : " << thread_count << "\n";
	obj_scale = (scalar) 1 / gtk_range_get_value((GtkRange*) obj_fraction_scale);
	std::cout << "scale : " << obj_scale << "\n";
	
	
		scene.FreeImport();
	if(Import::OBJ(buffer, &scene, obj_scale)){
		strcpy(obj_path, buffer);
		
		Allocator::FreeAll();
		std::cout << "BVH\n";
		std::cout << "building...\n";
		int stamp = clock();
		std::cout << "primitve count: " << scene.primitive_count << "\n";
		accelerator.Build(scene.primitives, scene.primitive_count);
		std::cout << "finished in " << (scalar)(clock()-stamp)/CLOCKS_PER_SEC << "\n";
		scene.accelerator = &accelerator;
		prepared = true;
	}
	
	OpenGL::triangles_to_draw_count = scene.primitive_count;
	OpenGL::triangles_to_draw = (Triangle*)scene.primitives;
	OpenGL::preview_camera = &camera;
}

static void focus_func (GtkWidget *widget,
             gpointer   data)
{
	if(prepared){
		Ray ray;
		scalar buff[2];
		buff[0] = camera.aperture_radius;
		buff[1] = camera.focus_distance;
		camera.aperture_radius = 0;
		camera.focus_distance = 10;
		
		camera.SampleRay(focus_x_pixel, focus_y_pixel, &ray);
		camera.aperture_radius = buff[0];
		camera.focus_distance = buff[1];
		
		scalar min = std::numeric_limits<scalar>::max();
		Hit hit;
		hit.hit = false;
		ray.UpdateInverse();
		accelerator.IntersectRayLoop(&ray, &min, &hit, scene.primitives, false);

		gtk_range_set_value((GtkRange*) focus_scale, hit.t);
	}
	
}


Triangle * select_func(int x, int y){
	
	if(prepared){
		Ray ray;
		Hit hit;
		scalar buff[2];
		buff[0] = camera.aperture_radius;
		buff[1] = camera.focus_distance;
		camera.aperture_radius = 0;
		camera.focus_distance = 10;
		
		camera.SampleRay(x, y, &ray);
		camera.aperture_radius = buff[0];
		camera.focus_distance = buff[1];
		
		scalar min = std::numeric_limits<scalar>::max();
        ray.UpdateInverse();
		accelerator.IntersectRayLoop(&ray, &min, &hit, scene.primitives, false);
		if(hit.hit){
			gtk_label_set_text((GtkLabel *)material_label, ((Triangle*)hit.prim)->material->name);
			std::cout << ((Triangle*)hit.prim)->material->name << "\n";
			return (Triangle *) hit.prim;
		}
		else{
			gtk_label_set_text((GtkLabel *)material_label, "NULL");
            return NULL;
		}
	}
	return NULL;
}

static void update_materials(){
	GdkRGBA rgba;
	
	if(selected_triangle != NULL and not lock_material){
		selected_triangle->material->roughness         = (scalar) gtk_range_get_value((GtkRange*) roughness_scale);
		selected_triangle->material->metallic          = (scalar) gtk_range_get_value((GtkRange*) metallic_scale);
		selected_triangle->material->transmission      = (scalar) gtk_range_get_value((GtkRange*) transmission_scale);
		selected_triangle->material->ior               = (scalar) gtk_range_get_value((GtkRange*) ior_scale);
		selected_triangle->material->emission_strength = (scalar) gtk_range_get_value((GtkRange*) emission_scale);
		
		gtk_color_chooser_get_rgba((GtkColorChooser *) material_color_button, &rgba);
		selected_triangle->material->color[0] = rgba.red;
		selected_triangle->material->color[1] = rgba.green;
		selected_triangle->material->color[2] = rgba.blue;
		
		gtk_color_chooser_get_rgba((GtkColorChooser *) material_emission_color_button, &rgba);
		selected_triangle->material->emission_color[0] = rgba.red;
		selected_triangle->material->emission_color[1] = rgba.green;
		selected_triangle->material->emission_color[2] = rgba.blue;
		
	}
}

static void set_material_scales(){
	GdkRGBA rgba;
	
	lock_material = true;
	if(selected_triangle != NULL){
		gtk_range_set_value((GtkRange*) roughness_scale   , selected_triangle->material->roughness);
		gtk_range_set_value((GtkRange*) metallic_scale    , selected_triangle->material->metallic);
		gtk_range_set_value((GtkRange*) transmission_scale, selected_triangle->material->transmission);
		gtk_range_set_value((GtkRange*) ior_scale         , selected_triangle->material->ior);
		gtk_range_set_value((GtkRange*) emission_scale    , selected_triangle->material->emission_strength);
		
		rgba.red   = selected_triangle->material->color[0];
		rgba.green = selected_triangle->material->color[1];
		rgba.blue  = selected_triangle->material->color[2];
		rgba.alpha = 1;
		gtk_color_chooser_set_rgba((GtkColorChooser *) material_color_button, &rgba);
		
		rgba.red   = selected_triangle->material->emission_color[0];
		rgba.green = selected_triangle->material->emission_color[1];
		rgba.blue  = selected_triangle->material->emission_color[2];
		rgba.alpha = 1;
		gtk_color_chooser_set_rgba((GtkColorChooser *) material_emission_color_button, &rgba);
	}
	lock_material = false;
}

static void glut_mouse_func(int button, int status, int x, int y){
	if(button == GLUT_RIGHT_BUTTON and status==GLUT_DOWN){
		focus_x_pixel = x;
		focus_y_pixel = camera.res_y-y;
		
		gtk_range_set_value((GtkRange*) focus_x_scale, ((scalar)focus_x_pixel/camera.res_x)*2 - 1);
		gtk_range_set_value((GtkRange*) focus_y_scale, ((scalar)focus_y_pixel/camera.res_y)*2 - 1);
		
		focus_func(NULL, NULL);
	}
	if(button == GLUT_LEFT_BUTTON and status==GLUT_DOWN){
        selected_triangle = select_func(x, camera.res_y-y);
		set_material_scales();
	}
}

static void render_func (GtkWidget *widget,
             gpointer   data)
{
	
	
	
	if(gtk_toggle_button_get_active((GtkToggleButton*) bidirectional_button)){
		integrator = &bidirectional_integrator_exp;
	}
	else{
		integrator = &unidirectional_integrator;
	}
	
	if(gtk_toggle_button_get_active((GtkToggleButton*) mask_button) and selected_triangle != NULL){
		Render::mask = selected_triangle->material;
	}
	else{
		Render::mask = NULL;
	}
	
	Render::threads = gtk_spin_button_get_value_as_int((GtkSpinButton*) entry_thread);
	integrator->camera_depth = gtk_spin_button_get_value_as_int((GtkSpinButton*) entry_camera_depth);
	integrator->light_depth = gtk_spin_button_get_value_as_int((GtkSpinButton*) entry_light_depth);
	std::cout << "integrator->light_depth : " << integrator->light_depth << "\n";
	integrator->samples = gtk_spin_button_get_value_as_int((GtkSpinButton*) entry_samples);
	integrator->scene = &scene;
	integrator->mask = Render::mask;
	integrator->in_ior = 1;

	if(light[0] != NULL){
		std::free(light[0]);
	}
	scalar light_pos[3];
	light_pos[0] = (scalar) gtk_range_get_value((GtkRange*) sun_pos_x_scale);
	light_pos[1] = (scalar) gtk_range_get_value((GtkRange*) sun_pos_y_scale);
	light_pos[2] = (scalar) gtk_range_get_value((GtkRange*) sun_pos_z_scale);

	scalar light_color[3] = {
			1.0,
			0.9,
			1.0
	};

	if( gtk_toggle_button_get_active((GtkToggleButton*) sun_bool) ){
		bounding_sphere = BoundingSphere(scene.primitives, scene.primitive_count, 16);
		light[0] = new DirectionalLight((scalar) gtk_range_get_value((GtkRange*) sun_scale),
									 light_color,
									 light_pos,
									 &bounding_sphere);
	}
	else{
		light[0] = new PointLight((scalar) gtk_range_get_value((GtkRange*) sun_scale),
							   light_color,
							   light_pos);
	}

	camera_pos[0] = (scalar) gtk_range_get_value((GtkRange*) cam_x_scale);
	camera_pos[1] = (scalar) gtk_range_get_value((GtkRange*) cam_y_scale);
	camera_pos[2] = (scalar) gtk_range_get_value((GtkRange*) cam_z_scale);
	
	camera_target[0] = (scalar) gtk_range_get_value((GtkRange*) tar_x_scale);
	camera_target[1] = (scalar) gtk_range_get_value((GtkRange*) tar_y_scale);
	camera_target[2] = (scalar) gtk_range_get_value((GtkRange*) tar_z_scale);
	
	
	scene.bg[0] = (scalar) gtk_range_get_value((GtkRange*) bg_r_scale);
	scene.bg[1] = (scalar) gtk_range_get_value((GtkRange*) bg_g_scale);
	scene.bg[2] = (scalar) gtk_range_get_value((GtkRange*) bg_b_scale);

	pupil_size = gtk_range_get_value((GtkRange*) pupil_scale);
	focus_distance = gtk_range_get_value((GtkRange*) focus_scale);
	
	int res_buff[2];
	res_buff[0] = gtk_spin_button_get_value_as_int((GtkSpinButton*) entry_res_x);
	res_buff[1] = gtk_spin_button_get_value_as_int((GtkSpinButton*) entry_res_y);
	
	if(res_buff[0] != camera.res_x or res_buff[1] != camera.res_y){
		scene.cameras->Resize(res_buff[0], res_buff[1]);
		OpenGL::buffer = camera.renderbuffer;
		OpenGL::width = camera.res_x;
		OpenGL::height = camera.res_y;
	}
	
	
	camera.LookAt(focal_length, pupil_size, focus_distance, camera_pos, camera_target, camera_up);
	//Render::lattice_sampling = gtk_toggle_button_get_active((GtkToggleButton*) smart_sample_button);
	
	std::cout << "thread_count : " << thread_count << "\n";
	if(prepared){
		int stamp = clock();

		Render::integrator = integrator;

		Render::RenderImage();

		std::cout << "FINISHED RENDERING !!!\n";
		std::cout << "TIME : " << (scalar)(clock()-stamp)/CLOCKS_PER_SEC << "\n";
		std::cout << "BVH INTERSECTIONS : " << bvh_intersections << "\n";
		OpenGL::DrawImage();
		glutMainLoopEvent();
		
		std::cout << "aabb_intersections_success : " << aabb_intersections_success << "/" << aabb_intersection_tests << "\n";
	}
}




void glut_draw_all_func(){
	OpenGL::render_mesh = gtk_toggle_button_get_active((GtkToggleButton*) preview_button);
	
	camera_pos[0] = (scalar) gtk_range_get_value((GtkRange*) cam_x_scale);
	camera_pos[1] = (scalar) gtk_range_get_value((GtkRange*) cam_y_scale);
	camera_pos[2] = (scalar) gtk_range_get_value((GtkRange*) cam_z_scale);
	
	camera_target[0] = (scalar) gtk_range_get_value((GtkRange*) tar_x_scale);
	camera_target[1] = (scalar) gtk_range_get_value((GtkRange*) tar_y_scale);
	camera_target[2] = (scalar) gtk_range_get_value((GtkRange*) tar_z_scale);
	
	
	camera.LookAt(focal_length, pupil_size, focus_distance, camera_pos, camera_target, camera_up);
	
	OpenGL::DrawImage();
}

static void display_main_menu(){
	g_object_ref(active_menu);
	gtk_container_remove(GTK_CONTAINER (window), active_menu);
	gtk_container_add (GTK_CONTAINER (window), table);
	active_menu = table;
	
	gtk_widget_show_all (window);
}

static void display_material_editor(){
	g_object_ref(active_menu);
	gtk_container_remove(GTK_CONTAINER (window), active_menu);
	gtk_container_add (GTK_CONTAINER (window), material_editor);
	active_menu = material_editor;
	
	gtk_widget_show_all (window);
}



static void activate (GtkApplication* app,
          gpointer        user_data)
{
	GtkWidget *render_button;
	GtkWidget *prepare_button;
	
	table = gtk_grid_new();
	gtk_grid_set_column_homogeneous((GtkGrid*) table, true);
	gtk_grid_set_row_homogeneous((GtkGrid*) table, true);
	gtk_grid_set_column_spacing((GtkGrid*) table, 15);
	
	GtkWidget *path_label = gtk_label_new("Import File Path");
	GtkWidget *thread_label = gtk_label_new("Thread Count");
	GtkWidget *obj_fraction_label = gtk_label_new("Inverse Import Scale");
	
	GtkWidget *camera_depth_label = gtk_label_new("Camera Depth");
	GtkWidget *light_depth_label = gtk_label_new("Light Depth");
	
	GtkWidget *samples_label = gtk_label_new("Samples");
	GtkWidget *res_x_label = gtk_label_new("X Resolution");
	GtkWidget *res_y_label = gtk_label_new("Y Resolotion");
	GtkWidget *sun_label = gtk_label_new("Sun Intensity");

	GtkWidget *sun_x_label = gtk_label_new("Sun X");
	GtkWidget *sun_y_label = gtk_label_new("Sun Y");
	GtkWidget *sun_z_label = gtk_label_new("Sun Z");
	
	GtkWidget *cam_x_label = gtk_label_new("Cam X");
	GtkWidget *cam_y_label = gtk_label_new("Cam Y");
	GtkWidget *cam_z_label = gtk_label_new("Cam Z");
	
	GtkWidget *tar_x_label = gtk_label_new("Tar X");
	GtkWidget *tar_y_label = gtk_label_new("Tar Y");
	GtkWidget *tar_z_label = gtk_label_new("Tar Z");
	
	
	GtkWidget *bg_r_label = gtk_label_new("Background R");
	GtkWidget *bg_g_label = gtk_label_new("Background G");
	GtkWidget *bg_b_label = gtk_label_new("Background B");
	
	
	GtkWidget *pupil_label = gtk_label_new("Pupil Size");
	GtkWidget *focus_label = gtk_label_new("Focus Distance");
	
	GtkWidget *focus_x_label = gtk_label_new("Autofocus X");
	GtkWidget *focus_y_label = gtk_label_new("Autofocus Y");

	entry_path = gtk_entry_new(); gtk_entry_set_text((GtkEntry*) entry_path, "gi_box.obj");
	entry_thread = gtk_spin_button_new_with_range(1, 96, 1); gtk_spin_button_set_value((GtkSpinButton*) entry_thread, 8);
	obj_fraction_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.1, 1000, 0.01); gtk_range_set_value((GtkRange*) obj_fraction_scale, 1);
	
	entry_camera_depth = gtk_spin_button_new_with_range(1, 128, 1);
	entry_light_depth = gtk_spin_button_new_with_range(0, 128, 1);
	
	entry_samples = gtk_spin_button_new_with_range(1, 1024, 1);
	sun_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, 10000, 0.01); gtk_range_set_value((GtkRange*)sun_scale, 1);
	sun_pos_x_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -25, 25, 0.01); gtk_range_set_value((GtkRange*)sun_pos_x_scale, 1);
	sun_pos_y_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -25, 25, 0.01); gtk_range_set_value((GtkRange*)sun_pos_y_scale, 1);
	sun_pos_z_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -25, 25, 0.01); gtk_range_set_value((GtkRange*)sun_pos_z_scale, 1);
	
	cam_x_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -25, 25, 0.05); gtk_range_set_value((GtkRange*)cam_x_scale, 5);
	cam_y_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -25, 25, 0.05); gtk_range_set_value((GtkRange*)cam_y_scale, 5);
	cam_z_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -25, 25, 0.05); gtk_range_set_value((GtkRange*)cam_z_scale, 5);
	
	tar_x_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -25, 25, 0.05); gtk_range_set_value((GtkRange*)tar_x_scale, 0);
	tar_y_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -25, 25, 0.05); gtk_range_set_value((GtkRange*)tar_y_scale, 0);
	tar_z_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -25, 25, 0.05); gtk_range_set_value((GtkRange*)tar_z_scale, 0);
	
	pupil_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 3, 0.001); gtk_range_set_value((GtkRange*)pupil_scale, 0.01);
	focus_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.01, 30, 0.01); gtk_range_set_value((GtkRange*)focus_scale, 7.5);
	
	bg_r_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01); gtk_range_set_value((GtkRange*)bg_r_scale, 0.3);
	bg_g_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01); gtk_range_set_value((GtkRange*)bg_g_scale, 0.4);
	bg_b_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1, 0.01); gtk_range_set_value((GtkRange*)bg_b_scale, 0.6);
	
	
	entry_res_x = gtk_spin_button_new_with_range(1, 1920*2, 1); gtk_spin_button_set_value((GtkSpinButton*) entry_res_x, 720);
	entry_res_y = gtk_spin_button_new_with_range(1, 1080*2, 1); gtk_spin_button_set_value((GtkSpinButton*) entry_res_y, 540);
	
	
	focus_x_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -1, 1, 0.01); gtk_range_set_value((GtkRange*)focus_x_scale, 0);
	focus_y_scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, -1, 1, 0.01); gtk_range_set_value((GtkRange*)focus_y_scale, 0);
	
	gtk_grid_attach((GtkGrid*)table, path_label, 1, 0, 1, 1);
	gtk_grid_attach((GtkGrid*)table, thread_label, 1, 1, 1, 1);
	gtk_grid_attach((GtkGrid*)table, obj_fraction_label, 1, 2, 1, 1);
	
	gtk_grid_attach((GtkGrid*)table, camera_depth_label, 1, 3, 1, 1);
	gtk_grid_attach((GtkGrid*)table, light_depth_label, 1, 4, 1, 1);
	
	gtk_grid_attach((GtkGrid*)table, samples_label, 1, 5, 1, 1);
	gtk_grid_attach((GtkGrid*)table, res_x_label, 1, 6, 1, 1);
	gtk_grid_attach((GtkGrid*)table, res_y_label, 1, 7, 1, 1);
	gtk_grid_attach((GtkGrid*)table, sun_label, 1, 8, 1, 1);

	gtk_grid_attach((GtkGrid*)table, sun_x_label, 1, 9 , 1, 1);
	gtk_grid_attach((GtkGrid*)table, sun_y_label, 1, 10, 1, 1);
	gtk_grid_attach((GtkGrid*)table, sun_z_label, 1, 11, 1, 1);
	
	gtk_grid_attach((GtkGrid*)table, entry_path, 2, 0, 1, 1);
	gtk_grid_attach((GtkGrid*)table, entry_thread, 2, 1, 1, 1);
	gtk_grid_attach((GtkGrid*)table, obj_fraction_scale, 2, 2, 1, 1);
	
	gtk_grid_attach((GtkGrid*)table, entry_camera_depth, 2, 3, 1, 1);
	gtk_grid_attach((GtkGrid*)table, entry_light_depth, 2, 4, 1, 1);
	
	gtk_grid_attach((GtkGrid*)table, entry_samples, 2, 5, 1, 1);
	gtk_grid_attach((GtkGrid*)table, entry_res_x, 2, 6, 1, 1);
	gtk_grid_attach((GtkGrid*)table, entry_res_y, 2, 7, 1, 1);

	gtk_grid_attach((GtkGrid*)table, sun_scale, 2, 8, 1, 1);

	gtk_grid_attach((GtkGrid*)table, sun_pos_x_scale, 2, 9 , 1, 1);
	gtk_grid_attach((GtkGrid*)table, sun_pos_y_scale, 2, 10, 1, 1);
	gtk_grid_attach((GtkGrid*)table, sun_pos_z_scale, 2, 11, 1, 1);

	sun_bool = gtk_check_button_new_with_label("DIRECTIONAL");
	gtk_grid_attach((GtkGrid*)table, sun_bool, 2, 12, 1, 1);

	gtk_grid_attach((GtkGrid*)table, cam_x_label, 3, 0, 1, 1);
	gtk_grid_attach((GtkGrid*)table, cam_y_label, 3, 1, 1, 1);
	gtk_grid_attach((GtkGrid*)table, cam_z_label, 3, 2, 1, 1);
	
	gtk_grid_attach((GtkGrid*)table, cam_x_scale, 4, 0, 1, 1);g_signal_connect (cam_x_scale, "value-changed", G_CALLBACK (glut_draw_all_func), NULL);
	gtk_grid_attach((GtkGrid*)table, cam_y_scale, 4, 1, 1, 1);g_signal_connect (cam_y_scale, "value-changed", G_CALLBACK (glut_draw_all_func), NULL);
	gtk_grid_attach((GtkGrid*)table, cam_z_scale, 4, 2, 1, 1);g_signal_connect (cam_z_scale, "value-changed", G_CALLBACK (glut_draw_all_func), NULL);
	
	
	gtk_grid_attach((GtkGrid*)table, tar_x_label, 3, 3, 1, 1);
	gtk_grid_attach((GtkGrid*)table, tar_y_label, 3, 4, 1, 1);
	gtk_grid_attach((GtkGrid*)table, tar_z_label, 3, 5, 1, 1);
	
	gtk_grid_attach((GtkGrid*)table, pupil_label, 3, 6, 1, 1);
	gtk_grid_attach((GtkGrid*)table, focus_label, 3, 7, 1, 1);
	gtk_grid_attach((GtkGrid*)table, focus_x_label, 3, 8, 1, 1);
	gtk_grid_attach((GtkGrid*)table, focus_y_label, 3, 9, 1, 1);
	
	gtk_grid_attach((GtkGrid*)table, tar_x_scale, 4, 3, 1, 1);g_signal_connect (tar_x_scale, "value-changed", G_CALLBACK (glut_draw_all_func), NULL);
	gtk_grid_attach((GtkGrid*)table, tar_y_scale, 4, 4, 1, 1);g_signal_connect (tar_y_scale, "value-changed", G_CALLBACK (glut_draw_all_func), NULL);
	gtk_grid_attach((GtkGrid*)table, tar_z_scale, 4, 5, 1, 1);g_signal_connect (tar_z_scale, "value-changed", G_CALLBACK (glut_draw_all_func), NULL);
	
	gtk_grid_attach((GtkGrid*)table, pupil_scale, 4, 6, 1, 1);
	gtk_grid_attach((GtkGrid*)table, focus_scale, 4, 7, 1, 1);
	gtk_grid_attach((GtkGrid*)table, focus_x_scale, 4, 8, 1, 1);
	gtk_grid_attach((GtkGrid*)table, focus_y_scale, 4, 9, 1, 1);
	
	gtk_grid_attach((GtkGrid*)table, bg_r_scale, 4, 10, 1, 1);
	gtk_grid_attach((GtkGrid*)table, bg_g_scale, 4, 11, 1, 1);
	gtk_grid_attach((GtkGrid*)table, bg_b_scale, 4, 12, 1, 1);
	
	
	
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Settings");
	//gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  
	//button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	//gtk_container_add(GTK_CONTAINER (window), button_box);
	
	render_button = gtk_button_new_with_label("RENDER");
	g_signal_connect(render_button, "clicked", G_CALLBACK(render_func), NULL);
	gtk_grid_attach((GtkGrid*)table, render_button, 0, 1, 1, 1);
	
	prepare_button = gtk_button_new_with_label("PREPARE");
	g_signal_connect(prepare_button, "clicked", G_CALLBACK(prepare_func), NULL);
	gtk_grid_attach((GtkGrid*)table, prepare_button, 0, 0, 1, 1);
	
	focus_button = gtk_button_new_with_label("AUTOFOCUS");
	g_signal_connect(focus_button, "clicked", G_CALLBACK(focus_func), NULL);
	gtk_grid_attach((GtkGrid*)table, focus_button, 0, 2, 1, 1);
	
	smart_sample_button = gtk_check_button_new_with_label("LATTICE SAMPLING");
	gtk_grid_attach((GtkGrid*)table, smart_sample_button, 0, 3, 1, 1);
	
	preview_button = gtk_check_button_new_with_label("ENABLE OpenGL PREVIEW");g_signal_connect (preview_button, "toggled", G_CALLBACK (glut_draw_all_func), NULL);
	gtk_grid_attach((GtkGrid*)table, preview_button, 0, 4, 1, 1);
	bidirectional_button = gtk_check_button_new_with_label("ENABLE Bidirectional Pathtracing");
	gtk_grid_attach((GtkGrid*)table, bidirectional_button, 0, 5, 1, 1);
	
	GtkWidget *material_editor_button = gtk_button_new_with_label("EDIT materials");
	g_signal_connect(material_editor_button, "clicked", G_CALLBACK(display_material_editor), NULL);
	gtk_grid_attach((GtkGrid*)table, material_editor_button, 0, 6, 1, 1);



	//design material editor
	
	material_editor = gtk_grid_new();
	material_label = gtk_label_new("Placeholder");
	gtk_grid_set_column_homogeneous((GtkGrid*) material_editor, true);
	gtk_grid_set_row_homogeneous((GtkGrid*) material_editor, true);
	gtk_grid_set_column_spacing((GtkGrid*) material_editor, 15);
	
	
	//add basic scales
	GtkWidget **scale;
	gtk_grid_attach((GtkGrid*)material_editor, material_label, 0, 0, 1, 1);
	
	scale = &roughness_scale;    *scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1,  0.005); gtk_range_set_value((GtkRange*)*scale, 0.50);
	g_signal_connect (*scale, "value-changed", G_CALLBACK (update_materials), NULL);
	gtk_grid_attach((GtkGrid*)material_editor, *scale, 2, 0, 1, 1);
	
	scale = &metallic_scale;     *scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1,  0.005); gtk_range_set_value((GtkRange*)*scale, 0.50);
	g_signal_connect (*scale, "value-changed", G_CALLBACK (update_materials), NULL);
	gtk_grid_attach((GtkGrid*)material_editor, *scale, 2, 1, 1, 1);
	
	scale = &transmission_scale; *scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 1,  0.005); gtk_range_set_value((GtkRange*)*scale, 0.00);
	g_signal_connect (*scale, "value-changed", G_CALLBACK (update_materials), NULL);
	gtk_grid_attach((GtkGrid*)material_editor, *scale, 2, 2, 1, 1);
	
	scale = &ior_scale;          *scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 10, 0.005); gtk_range_set_value((GtkRange*)*scale, 1.45);
	g_signal_connect (*scale, "value-changed", G_CALLBACK (update_materials), NULL);
	gtk_grid_attach((GtkGrid*)material_editor, *scale, 2, 3, 1, 1);
	
	
	material_color_button = gtk_color_button_new();
	g_signal_connect (material_color_button, "color-set", G_CALLBACK (update_materials), NULL);
	gtk_grid_attach((GtkGrid*)material_editor, material_color_button, 2, 4, 1, 1);
	
	material_emission_color_button = gtk_color_button_new();
	g_signal_connect (material_emission_color_button, "color-set", G_CALLBACK (update_materials), NULL);
	gtk_grid_attach((GtkGrid*)material_editor, material_emission_color_button, 2, 5, 1, 1);
	
	
	scale = &emission_scale;          *scale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 10, 0.005); gtk_range_set_value((GtkRange*)*scale, 1.45);
	g_signal_connect (*scale, "value-changed", G_CALLBACK (update_materials), NULL);
	gtk_grid_attach((GtkGrid*)material_editor, *scale, 2, 6, 1, 1);
	
	//add lables :
	gtk_grid_attach((GtkGrid*)material_editor, gtk_label_new("roughness"),         1, 0, 1, 1);
	gtk_grid_attach((GtkGrid*)material_editor, gtk_label_new("metallic"),          1, 1, 1, 1);
	gtk_grid_attach((GtkGrid*)material_editor, gtk_label_new("transmission"),      1, 2, 1, 1);
	gtk_grid_attach((GtkGrid*)material_editor, gtk_label_new("ior"),               1, 3, 1, 1);
	gtk_grid_attach((GtkGrid*)material_editor, gtk_label_new("color"),             1, 4, 1, 1);
	gtk_grid_attach((GtkGrid*)material_editor, gtk_label_new("emission color"),    1, 5, 1, 1);
	gtk_grid_attach((GtkGrid*)material_editor, gtk_label_new("emission strength"), 1, 6, 1, 1);
	
	GtkWidget *main_menu_button = gtk_button_new_with_label("MAIN MENU");
	g_signal_connect(main_menu_button, "clicked", G_CALLBACK(display_main_menu), NULL);
	gtk_grid_attach((GtkGrid*)material_editor, main_menu_button, 0, 1, 1, 1);
	
	GtkWidget * render_button2 = gtk_button_new_with_label("RENDER");
	g_signal_connect(render_button2, "clicked", G_CALLBACK(render_func), NULL);
	gtk_grid_attach((GtkGrid*)material_editor, render_button2,    0, 2, 1, 1);
	
	mask_button = gtk_check_button_new_with_label("Isolate material");
	gtk_grid_attach((GtkGrid*)material_editor, mask_button, 0, 3, 1, 1);
	
	list_box_container = gtk_scrolled_window_new(NULL, NULL);
	texture_list_box = gtk_list_box_new();
	gtk_container_add (GTK_CONTAINER (list_box_container), texture_list_box);
	gtk_grid_attach((GtkGrid*)material_editor, list_box_container, 3, 0, 1, 1);
	
	checker = new ProceduralTexture2D();
	copyStr128(checker->name, (char*) "checker");
	image_texture = new ImageTexture();
	image_texture->ImportPNG((char*)"abstract.png");
	copyStr128(image_texture->name, (char*)"abstract.png");
	
	
	add_texture_to_list(NULL, (char*)  "NULL");
	add_texture_to_list(image_texture, image_texture->name);
	add_texture_to_list(checker, checker->name);
	
	
	gtk_container_add (GTK_CONTAINER (window), table);
	active_menu = table;
	//gtk_container_add (GTK_CONTAINER (window), material_editor);
	
	//gtk_container_add (GTK_CONTAINER (window), table);
	
	gtk_widget_show_all (window);
}



int main(int argc, char ** argv){

	bidirectional_integrator_exp = BidirectionalIntegrator_exp();
	lock_material = false;
	selected_triangle = NULL;
	texture_count = 0;
	
	OpenGL::triangles_to_draw_count = 0;
	Parameters::argc = argc;
	Parameters::argv = argv;
	
	prepared = false;
	
	scalar light_pos[3] = {2, -2, 2};
	scalar light_color[3] = {1, 0.9, 0.9};
	scalar light_intensity = 2;
	bool sun = true;
	scalar sun_direction[3] = {-1, 1, -1.6};
	//light = Light(light_pos, light_color, light_intensity, sun, sun_direction);

	light_pos[0] = -1;
	light_pos[1] = -1;
	light_pos[2] = -1;
	light[0] = new DirectionalLight(light_intensity*100, light_color, light_pos, &bounding_sphere);

	light_color[0] = 0.5;
	light_color[1] = 0.8;
	light_color[2] = 0.1;
	//light[1] = Light(light_pos, light_color, light_intensity*100, false, sun_direction);;
	
	char input[128];
	
	camera_up[0] = 0;
	camera_up[1] = 0;
	camera_up[2] = 1;
	
	focal_length = 50;
	camera = Camera(720, 540);
	/*
	std::cout << "EDIT RESOLUTION ? [Y/n] : ";
	std::cin >> input;
	if(input[0] != 'n' and input[0] != 'N'){
		std::cout << "RES X : ";
		std::cin >> camera_res_x;
		std::cout << "RES Y : ";
		std::cin >> camera_res_y;
	}
	*/
	
	camera.LookAt(focal_length, pupil_size, focus_distance, camera_pos, camera_target, camera_up);
	
	
	
	scene = Scene(light, 1, &camera, 1, 0);
	
	strcpy(obj_path, "E:/GitProjects/CppPathtracer/src/build/bin/Scenes/gi_box.obj");
	obj_scale = 1;
	
	/*
	std::cout << "EDIT IMPORT SETTINGS ? [Y/n] : ";
	std::cin >> input;
	
	if(input[0] != 'n' and input[0] != 'N'){
		std::cout << "OBJ PATH : ";
		std::cin >> obj_path;
		
		std::cout << "OBJ SCALE : ";
		std::cin >> obj_scale;
	}
	*/
	
	std::cout << "OpenGL\n";
	OpenGL::buffer = scene.cameras[scene.active_camera].renderbuffer;
	OpenGL::width  = scene.cameras[scene.active_camera].res_x;
	OpenGL::height = scene.cameras[scene.active_camera].res_y;
	OpenGL::InitOpenGL(argc, argv, camera.res_x, camera.res_y, (char *) "CPP-Pathtracer-Rewrite v1.7a");
	
	OpenGL::id = glutCreateWindow( (char *) "CPP-Pathtracer-Rewrite v1.7a");
	glClearColor(0, 0.3, 0.3, 1);
	gluOrtho2D(0, 1, 0, 1);		
	
	OpenGL::InitRenderTexture();
	
	glutDisplayFunc(OpenGL::DrawImage);
	//glutResizeFunc(glut_draw_all_func);
	//glutIdleFunc(glut_draw_all_func);
	
	glutMouseFunc(glut_mouse_func);
	
	int i = 0;
	
	while(i < camera.res_x*camera.res_y*3){
		OpenGL::buffer[i] = fastRand();
		++i;
	}
	OpenGL::DrawImage();
	glutMainLoopEvent();
	
	
	
	std::cout << "start glut main loop\n";
	std::thread glut_main_loop_thread = std::thread(glutMainLoop);
	
	
	
	GtkApplication *app;
	int status;
	
	app = gtk_application_new ("org.gtk.settings", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	
	
	glut_main_loop_thread.join();
	/*
	stamp = clock();
	
	Render::Render(thread_count, 4096, &integrator, &scene);
	
	std::cout << "FINISHED RENDERING !!!\n";
	std::cout << "TIME : " << (scalar)(clock()-stamp)/CLOCKS_PER_SEC << "\n";
	std::cout << "BVH INTERSECTIONS : " << bvh_intersections << "\n";
	OpenGL::DrawImage();
	glutMainLoopEvent();
	
	std::cout << "aabb_intersections_success : " << aabb_intersections_success << "/" << aabb_intersection_tests << "\n";
	*/
	
	
	
	return true;
}