#include "CfgManager.h"
#include "CfgManagerT.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


//---Python bindings
PYBIND11_MODULE(cfgmanager, m) {
    pybind11::class_<CfgManager>(m, "CfgManager")
        .def(pybind11::init<const char*>())
        .def(pybind11::init())
        .def("GetOpt", &CfgManager::GetOpt<std::string>,
             "Get a single field from option as string",
             pybind11::arg("key"),
             pybind11::arg("opt") = 0
            )
        .def("GetDoubleOpt", &CfgManager::GetOpt<double>,
             "Get a single field from option as double",
             pybind11::arg("key"),
             pybind11::arg("opt") = 0
            )
        .def("GetVOpt", &CfgManager::GetOpt<std::vector<std::string> >,
             "Get all fields from option as strings",
             pybind11::arg("key"),
             pybind11::arg("opt") = 0
            )
        .def("GetVDoubleOpt", &CfgManager::GetOpt<std::vector<double> >,
             "Get all fields from option as doubles",
             pybind11::arg("key"),
             pybind11::arg("opt") = 0
            )
        .def("SetOpt", (void (CfgManager::*)(const char* , option_t&)) &CfgManager::SetOpt,
             "Set option",
             pybind11::arg("key"),
             pybind11::arg("v_opt"))
        .def("OptExist", &CfgManager::OptExist)
        .def("GetSubCfg", (CfgManager (CfgManager::*)(std::string) const) &CfgManager::GetSubCfg,
             "Get the a single cfg block",
             pybind11::arg("block"))
        .def("GetSubCfg", (CfgManager (CfgManager::*)(std::vector<std::string>) const) &CfgManager::GetSubCfg,
             "Get the a set of blocks",
             pybind11::arg("blocks"))
        .def("CompareOption", &CfgManager::CompareOption)
        .def("ParseConfigFile", (void (CfgManager::*)(const char*)) &CfgManager::ParseConfigFile,
             "Parse a cfg file",
             pybind11::arg("file"))
        .def("ParseConfigString", &CfgManager::ParseConfigString)
        .def("WriteToFile", &CfgManager::WriteToFile)
        .def("__repr__", (std::string (CfgManager::*)(std::string)) &CfgManager::AsString,
             "Convert cfg to plain string",
             pybind11::arg("option") = "");             
}
