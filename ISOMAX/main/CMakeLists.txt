# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 
  "../TopLevel/AppHW.c"
  "../TopLevel/AppMain.c"

  "../src/03_04_05_07_08_Base/AppIso_Impl_NetDl.c"
  "../src/06_VTC/AppIso_Impl_VTApp.c"
  "../src/10_TCC/AppIso_Impl_TCApp.c"
  "../src/10_TCC/AppIso_Impl_TDLCApp.c"
  "../src/12_Diagnostic/AppIso_Diagnostic.c"
  "../src/AddOn/AppInterface.c"
  "../src/AddOn/AppIso_Output.c"
  "../src/ISO_DesignerProjects/MyWorkspace1/MyProject1/Output/MyProject1.2.c"
  "../src/ISO_DesignerProjects/MyWorkspace1/MyProject1/Output/MyProject1.3.c"
  "../src/ISO_DesignerProjects/MyWorkspace1/MyProject1/Output/MyProject1.c"
  "../src/SpeedDistanceHelper/SpeedDistanceAreaHelper.c"
  "../src/GeometryHandling/GeometryHandling.c"
  "../src/General_Settings/General_Settings.c"
  "../src/IsoCommandECHelper/IsoCommandECHelper.c")
  
set(COMPONENT_ADD_INCLUDEDIRS 
  "."
  "../TopLevel"
  "../IsoConfig"
  "../lib_cci"
  "../src/AddOn"
  "../src/03_04_05_07_08_Base"
  "../src/06_VTC"
  "../src/SpeedDistanceHelper"
  "../src/IsoCommandECHelper"
  "../src/GeometryHandling"
  "../src/General_Settings"
  "../src/12_Diagnostic"
  "../src/10_TCC"
  "../src/ISO_DesignerProjects/MyWorkspace1/MyProject1/Output"
  "../AppMain")
  
  register_component()
  
  
  # Import the library, specifying a target name and the library path.
# The private dependencies of the library is also specified.
add_prebuilt_library(prebuilt_cci "../lib_cci/libprebuilt_cci.a")

# `main` calls a function from the library, so link it to `main`
target_link_libraries(${COMPONENT_LIB} PRIVATE prebuilt_cci)
