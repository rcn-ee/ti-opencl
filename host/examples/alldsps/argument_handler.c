    if ( argc < 4)
    {
        cerr << "Use: {cpu|dsp|both} devices workgroups " << endl;
        exit(EXIT_FAILURE);
    }

    /*-------------------------------------------------------------------------
    * handle command-line arguments
    *------------------------------------------------------------------------*/
    const std::string platformName(argv[1]);
    int   device_category = CL_DEVICE_TYPE_ALL;

    /*-------------------------------------------------------------------------
    * crudely parse the command line arguments
    *------------------------------------------------------------------------*/
    if      (platformName.compare("cpu")==0)  device_category = CL_DEVICE_TYPE_CPU;
    else if (platformName.compare("dsp")==0)  device_category = CL_DEVICE_TYPE_ACCELERATOR;
    else if (platformName.compare("both")==0) device_category = CL_DEVICE_TYPE_ALL;
    else    { cerr << "Invalid device type!" << endl; return(1); }


    const int NumDevices    = atoi(argv[2]);
    const int NumWorkGroups = atoi(argv[3]);
