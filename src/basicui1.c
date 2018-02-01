#include "basicui1.h"

#define SIZE_RECORDED 100

bool supported = false;
//char chrHello[50] = "<align=center>Hello World</align>";

float fltRecordAccX[SIZE_RECORDED]; // on enregistre pendant 1 minute avec 10 enregistrements par seconde (100ms)
float fltRecordAccY[SIZE_RECORDED];
float fltRecordAccZ[SIZE_RECORDED];
int intNbRecorded = 0;
bool gblFirstTime = 1;
bool gblConnected = 0;
CURL *curl;
CURLcode res;
char buffer[100];
bool gblStart = 0;
bool gblWouldStop = 0;


typedef struct appdata {
	sensor_h sensorA;
	sensor_listener_h listenerA;
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
	Evas_Object *button;
} appdata_s;

static void device_vibrate(int duration, int feedback)
{
	haptic_device_h haptic_handle;
	haptic_effect_h effect_handle;

	if(device_haptic_open(0, &haptic_handle) == DEVICE_ERROR_NONE)
	{
		LOGI("Connection to vibrator established");

		if(device_haptic_vibrate(haptic_handle, duration, feedback, &effect_handle) == DEVICE_ERROR_NONE)
			LOGI("Device vibrates!");

//		To stop vibration which are being played use below code with proper handles
//		if(device_haptic_stop(haptic_handle, effect_handle) == DEVICE_ERROR_NONE)
//			LOGI("Device vibration stopped");

//		When you decided not to use haptic anymore disconnect it
//		if(device_haptic_close(haptic_handle) == DEVICE_ERROR_NONE)
//			LOGI("Vibrator disconnected");
	}
}

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}



static char* read_file(const char* filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL)
        return NULL;
    fseek(fp, 0, SEEK_END);
    int bufsize = ftell(fp);
    rewind(fp);
    if (bufsize < 1)
        return NULL;
    char *buf = malloc(sizeof(char) * (bufsize));
    memset(buf, '\0', sizeof(buf));
    char str[200];
    while(fgets(str, 200, fp) != NULL) {
        dlog_print(DLOG_ERROR, "tag", "%s", str);
        sprintf(buf + strlen(buf), "%s", str);
    }
    fclose(fp);
    return buf;
}

void tryToConnect(void *data, int lintMSG)
{
	appdata_s *ad = data;
	/*
	 * connection au serveur curl
	 *
	 */
	char lchrURL[1000] = {0,};
	curl = curl_easy_init();
	if (curl)
	{
		switch (lintMSG)
		{
			case 0:
				snprintf(lchrURL,1000,"http://192.168.0.37:8080/watch/start/%s",read_file("/csa/imei/serialno.dat"));
				break;
			case 1:
				snprintf(lchrURL,1000,"http://192.168.0.37:8080/watch/recording/%s",read_file("/csa/imei/serialno.dat"));
				break;
			case 2:
				snprintf(lchrURL,1000,"http://192.168.0.37:8080/watch/okstop/%s",read_file("/csa/imei/serialno.dat"));
				break;
			case 3:
				snprintf(lchrURL,1000,"http://192.168.0.37:8080/watch/arrecording/%s",read_file("/csa/imei/serialno.dat"));
				break;
			case 4:
				snprintf(lchrURL,1000,"http://192.168.0.37:8080/watch/msgread/%s",read_file("/csa/imei/serialno.dat"));
				break;
			case 5:
				snprintf(lchrURL,1000,"http://192.168.0.37:8080/watch/canistop/%s",read_file("/csa/imei/serialno.dat"));
				break;
			default :
				snprintf(lchrURL,1000,"http://192.168.0.37:8080/watch/start/%s",read_file("/csa/imei/serialno.dat"));
		}
		curl_easy_setopt(curl, CURLOPT_URL, lchrURL);

		/* complete within 20 seconds */
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
		{
			dlog_print(DLOG_ERROR, "FLODVauche", "curl_easy_perform() failed: err = %s", curl_easy_strerror(res));
			res = curl_easy_perform(curl);
			elm_object_text_set(ad->button, "Réessayer");
			elm_object_text_set(ad->label, "Cannot connect to server !");
			gblConnected = FALSE;
		}
		else
		{
			switch (lintMSG)
			{
				case 0:
					elm_object_text_set(ad->button, "Démarrer !");
					elm_object_text_set(ad->label, "Vauché PTMS by FLOD");
					gblConnected = TRUE;
					device_vibrate(1000,100);
					break;
				case 1:
					elm_object_text_set(ad->label, "Recording ...");
					device_vibrate(100,100);
					break;
				case 2:
					gblWouldStop = 0;
					break;
				case 5:
					elm_object_text_set(ad->label, "Veuillez attendre ...");
					device_vibrate(100,100);
					break;
			}
		}
	}
	curl_easy_cleanup(curl);
	/*
	 * connection au serveur curl
	 *
	 */

}

static void create_base_gui(appdata_s *ad)
{
	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win))
	{
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	//ad->conform = elm_conformant_add(ad->win);
	ad->conform = elm_grid_add(ad->win);
	elm_grid_size_set(ad->conform,10, 10);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	//evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Label */
	ad->label = elm_label_add(ad->conform);
	elm_object_text_set(ad->label, "Connecting ...");
	//evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_grid_pack(ad->conform,ad->label,0,4,10,2);
	elm_object_style_set(ad->label, "slide_roll");
	elm_label_slide_duration_set(ad->label, 3);
	elm_label_slide_mode_set(ad->label, ELM_LABEL_SLIDE_MODE_ALWAYS);
	///elm_object_content_set(ad->conform, ad->label);
	//elm_win_resize_object_add(ad->win, ad->label);

	/* Bouton */
	ad->button = elm_button_add(ad->conform);
	elm_grid_pack(ad->conform, ad->button,0,6,10,4);


	tryToConnect(ad,0);

//	elm_object_style_set(ad->button, "circle");

	elm_object_style_set(ad->button, "bottom");


	elm_object_content_set(ad->conform, ad->button);
	evas_object_smart_callback_add(ad->button, "clicked", clicked_cb, ad);
	//elm_win_resize_object_add(ad->win, ad->button);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);

	evas_object_show(ad->button);

	evas_object_show(ad->label);

}

void clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (gblConnected)
	{
			appdata_s *ad = data;
		    if (gblStart)
		    {
		    	if (gblWouldStop)
		    	{
		    		gblStart = 0;
					sensor_listener_stop(ad->listenerA);
					dlog_print(DLOG_INFO, "FLODVauche", "FLOD : Stop\n");
					elm_object_text_set(ad->label, "Vauché PTMS by FLOD");
					elm_object_text_set(ad->button, "Démarre !");
					intNbRecorded = 0;
					gblFirstTime = 1;
					tryToConnect(data,2);
		    	}
		    	else
		    	{
		    		tryToConnect(data,5);
		    	}

		    }
		    else
		    {
		    	gblStart = 1;
		    	sensor_listener_start(ad->listenerA);
		    	dlog_print(DLOG_INFO, "FLODVauche", "FLOD : Démarre la lecture des ACC\n");
		    	elm_object_text_set(ad->button, "Stop !");
		    	tryToConnect(data,1);
		    }
	}
	else
	{
		appdata_s *ad = data;
		elm_object_text_set(ad->label, "connexion ...");
		tryToConnect(data,0);

	}
}

void WriteBFile(void *user_data)
{
	char write_filepath[1000] = {0,};
	char *resource_path = app_get_data_path(); // get the application data directory path
	if(resource_path && gblFirstTime)
	{
		//appdata_s *ad = user_data;
		FILE *fp;

		snprintf(write_filepath,1000,"%s%s",resource_path,"fileAccX.dat");
		fp = fopen(write_filepath,"w");
		float x = 0;
		fwrite(fltRecordAccX , sizeof(x) , SIZE_RECORDED , fp );
		fclose(fp);

		snprintf(write_filepath,1000,"%s%s",resource_path,"fileAccY.dat");
		fp = fopen(write_filepath,"w");
		fwrite(fltRecordAccY , sizeof(x) , SIZE_RECORDED , fp );
		fclose(fp);

		snprintf(write_filepath,1000,"%s%s",resource_path,"fileAccZ.dat");
		fp = fopen(write_filepath,"w");
		fwrite(fltRecordAccZ , sizeof(x) , SIZE_RECORDED , fp );
		fclose(fp);


		//elm_object_text_set(ad->label, chrHello);

		//device_vibrate(1000,100);
		gblFirstTime = 0;

		int lintLimit = 0;
		struct curl_httppost* post = NULL;
		struct curl_httppost* last = NULL;

		//while ((res != CURLE_OK & lintLimit<7) | (lintLimit==0))
		{
			curl = curl_easy_init();
			if (curl)
			{
				lintLimit++;
				int error;
				error = device_power_request_lock(POWER_LOCK_CPU, 12000);
				device_power_request_lock(POWER_LOCK_DISPLAY, 5000);

				//defining the landing URL
				//curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.0.37:8888/upload");
				//Avec Linux Mint l'URL est différente : 10.42.0.1:8888
				curl_easy_setopt(curl, CURLOPT_URL, "http://10.42.0.1:8888/upload");
				//curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.0.16:8888/upload");

				snprintf(write_filepath,1000,"%s%s",resource_path,"fileAccZ.dat");

				/* Add simple file section
				post = NULL;
				last = NULL; */

				 /*
				  * read_file("/csa/imei/serialno.dat")
				  *
				  */
				 curl_formadd(&post, &last, CURLFORM_COPYNAME, "serialno",
							  CURLFORM_COPYCONTENTS,read_file("/csa/imei/serialno.dat"), CURLFORM_END);

				 curl_formadd(&post, &last,
						 CURLFORM_COPYNAME, "file", CURLFORM_FILE ,write_filepath, CURLFORM_END);

				 snprintf(write_filepath,1000,"%s%s",resource_path,"fileAccY.dat");
				/* Add simple file section */
				 curl_formadd(&post, &last,
						 CURLFORM_COPYNAME, "file", CURLFORM_FILE ,write_filepath, CURLFORM_END);

				snprintf(write_filepath,1000,"%s%s",resource_path,"fileAccX.dat");
				/* Add simple file section */
				 curl_formadd(&post, &last,
						 CURLFORM_COPYNAME, "file", CURLFORM_FILE ,write_filepath, CURLFORM_END);


				/* Now specify the POST data */
				curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
				//curl_easy_setopt(curl, CURLOPT_POST,1);

				if(res != CURLE_OK)
					dlog_print(DLOG_ERROR, "FLODVauche", "curl WRITEDATA failed: err = %s", curl_easy_strerror(res));

				curl_easy_setopt(curl, CURLOPT_TIMEOUT, 4L);

				/* Perform the request, res will get the return code */
				res = curl_easy_perform(curl);
				 /* Check for errors */

				if (res != CURLE_OK & lintLimit<7)
				{
					dlog_print(DLOG_ERROR, "FLODVauche", "curl_easy_perform() failed: err = %s", curl_easy_strerror(res));
					res = curl_easy_perform(curl);
					lintLimit++;
				}
				/* always cleanup */
				curl_formfree(post);
			}
			curl_easy_cleanup(curl);

	    	intNbRecorded = 0;
	    	gblFirstTime = 1;

		}
		free(resource_path);

	}
}

/* Define callback */
void sensorA_event(sensor_h sensor, sensor_event_s *sensor_data, void *user_data)
{
    /*
       If a callback is used to listen for different sensor types,
       it can check the sensor type
    */
	//appdata_s *ad = user_data;
    sensor_type_e type;
    sensor_get_type(sensor, &type);

    if (type == SENSOR_ACCELEROMETER)
    {
        //unsigned long long timestamp = sensor_data->timestamp;

    	//int accuracy = sensor_data->accuracy;
        float x = sensor_data->values[0];
        float y = sensor_data->values[1];
        float z = sensor_data->values[2];

        if (intNbRecorded<SIZE_RECORDED)
        {
        	fltRecordAccX[intNbRecorded] = x;
        	fltRecordAccY[intNbRecorded] = y;
        	fltRecordAccZ[intNbRecorded] = z;
        	intNbRecorded++;

        	//Affichage des données pour vérifier que le programme tourne bien,
        	// Je le mets en commentaire car ça prend peut-être un peu de resources
        	//sprintf(chrHello, "<br/><br/><align=center> %f !</align>", x);
        	//dlog_print(DLOG_INFO, "FLODVauche", "acceleration are X = %lf",x);
        	//elm_object_text_set(ad->label, chrHello);
        }
        else if (intNbRecorded==SIZE_RECORDED)
        	WriteBFile(user_data);
        //strcpy(chrHello, x);
    }

}


static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	sensor_is_supported(SENSOR_ACCELEROMETER, &supported);
	if (!supported) {
	    /* Accelerometer is not supported on the current device */
		//strcpy(chrHello, "<align=center>not OK</align>");
		dlog_print(DLOG_ERROR, LOG_TAG, "FLOD :Accelerometer is not supported! ...");
	}
	else {
		//strcpy(chrHello, "<align=center>Yes we can !</align>");
		dlog_print(DLOG_INFO, LOG_TAG, "FLOD : Accelerometer is supported! ...");
		//float x = 2.14;
		//sprintf(chrHello, "<align=center> %f !</align>", x);
		sensor_get_default_sensor(SENSOR_ACCELEROMETER, &ad->sensorA);
		sensor_create_listener(ad->sensorA, &ad->listenerA);

		/* Register callback */
		sensor_listener_set_event_cb(ad->listenerA, 100, sensorA_event, data);

		// To listen for sensor events, start the listener:
		//sensor_listener_start(ad->listenerA);

		dlog_print(DLOG_INFO, "FLODVauche", "acceleration are X = %f",8);

		/* By default,
		 * the listener automatically stops listening for the sensor data,
		 * if the display is switched off or the device goes to the power-save mode.
		 * You can override such behavior:
		 */
		sensor_listener_set_attribute_int(ad->listenerA, SENSOR_ATTRIBUTE_PAUSE_POLICY, SENSOR_PAUSE_NONE);

		/*
		 * When the sensor data is no more necessary, stop the listener:
		 */
		//sensor_listener_stop(listener);

	}

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	//strcpy(chrHello, "<align=center>Turned !</align>");
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	curl_global_init(CURL_GLOBAL_ALL);

	appdata_s ad = {0,};
	dlog_print(DLOG_INFO, "FLODVauche", "Application created");
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	curl_global_cleanup();
	return ret;
}
