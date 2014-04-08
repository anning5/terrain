////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// include all examples

#include "../../platform/platform.h"
#include"helpers/camera_control.h"
//#include <string>
//#include <fstream>
#include <time.h>
/*
#include "helpers/font_helper.h"
//*/

//
#include "terrain/terrain_app.h"



namespace octet {
  static app *app_factory(const char *name, int argc, char **argv) {
	  return new terrain_app(argc, argv);
  }

  inline void run_examples(int argc, char **argv) {
    //app_utils::prefix("../../");
    app::init_all(argc, argv);

    if (argc == 1) {
      app *myapp = app_factory("", argc, argv);
      // if you can't edit the debug arguments,
      // change the string above to another of the examples.
      myapp->init();
    } else {
      for (int i = 1; i != argc; ++i) {
        if (argv[i][0] != '-') {
          app *myapp = app_factory(argv[i], argc, argv);
          if (myapp) {
            myapp->init();
          }
        }
      }
    }

    app::run_all_apps();
  }
}
