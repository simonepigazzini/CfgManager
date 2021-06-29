#include "interface/CfgManager.h"
#include "interface/CfgManagerT.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


//---Python bindings
PYBIND11_MODULE(cfgmanager, m) {
    pybind11::class_<CfgManager>(m, "CfgManager")
        .def(pybind11::init<const char*>())
        .def(pybind11::init())
        .def("GetOpt", &CfgManager::GetOpt<std::string>)
        .def("GetOpt", &CfgManager::GetOpt<std::vector<std::string> >)
        .def("GetOpt", &CfgManager::GetOpt<double>)
        .def("GetOpt", &CfgManager::GetOpt<std::vector<double> >);
}
