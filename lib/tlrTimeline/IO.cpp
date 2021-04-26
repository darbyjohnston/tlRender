// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrTimeline/IO.h>

#include <tlrCore/File.h>

#if defined(TLR_ENABLE_PYTHON)
#include <Python.h>
#endif

#include <iostream>

namespace tlr
{
    namespace timeline
    {
#if defined(TLR_ENABLE_PYTHON)
        namespace
        {
            class PyObjectRef
            {
            public:
                PyObjectRef(PyObject* o) :
                    o(o)
                {
                    if (!o)
                    {
                        throw std::runtime_error("Python error");
                    }
                }

                ~PyObjectRef()
                {
                    Py_XDECREF(o);
                }

                PyObject* o = nullptr;

                operator PyObject* () const { return o; }
            };
        }
#endif

        otio::SerializableObject::Retainer<otio::Timeline> read(
            const std::string& fileName,
            otio::ErrorStatus* errorStatus)
        {
            otio::SerializableObject::Retainer<otio::Timeline> out;
#if defined(TLR_ENABLE_PYTHON)
            Py_Initialize();
            try
            {
                auto pyModule = PyObjectRef(PyImport_ImportModule("opentimelineio.adapters"));

                auto pyReadFromFile = PyObjectRef(PyObject_GetAttrString(pyModule, "read_from_file"));
                auto pyReadFromFileArgs = PyObjectRef(PyTuple_New(1));
                const std::string fileNameNormalized = file::normalize(fileName);
                auto pyReadFromFileArg = PyUnicode_FromStringAndSize(fileNameNormalized.c_str(), fileNameNormalized.size());
                if (!pyReadFromFileArg)
                {
                    throw std::runtime_error("Cannot create arg");
                }
                PyTuple_SetItem(pyReadFromFileArgs, 0, pyReadFromFileArg);
                auto pyTimeline = PyObjectRef(PyObject_CallObject(pyReadFromFile, pyReadFromFileArgs));

                auto pyToJSONString = PyObjectRef(PyObject_GetAttrString(pyTimeline, "to_json_string"));
                auto pyJSONString = PyObjectRef(PyObject_CallObject(pyToJSONString, NULL));
                out = otio::SerializableObject::Retainer<otio::Timeline>(
                    dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_string(
                        PyUnicode_AsUTF8AndSize(pyJSONString, NULL),
                        errorStatus)));
            }
            catch (const std::exception& e)
            {
                errorStatus->outcome = otio::ErrorStatus::Outcome::FILE_OPEN_FAILED;
                errorStatus->details = e.what();
            }
            if (PyErr_Occurred())
            {
                PyErr_Print();
            }
            Py_Finalize();
#else
            out = dynamic_cast<otio::Timeline*>(otio::Timeline::from_json_file(fileName, errorStatus));
#endif
            return out;
        }
    }
}
