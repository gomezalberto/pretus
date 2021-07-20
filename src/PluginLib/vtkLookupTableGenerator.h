#pragma once

#include <vtkColorSeries.h>
#include <vtkLookupTable.h>
#include <vtkVariantArray.h>
#include <vtkSmartPointer.h>

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>

/**
 * @brief The vtkLookupTableGenerator class
 * Borrowed from here:
 * https://www.paraview.org/Wiki/VTK/Examples/Cxx/Utilities/LUTUtilities
 */
class vtkLookupTableGenerator {

public:
    //-----------------------------------------------------------------------------
    //! Constructor.
    vtkLookupTableGenerator(){};

    //-----------------------------------------------------------------------------
    //! Destructor.
    ~vtkLookupTableGenerator(){};


    vtkSmartPointer<vtkLookupTable> GenerateLUT(int idx);


    //! Display the contents of the lookup table.
    /*!
     * @param lut - the lookup table.
     * @return a string containing the table data.
     */
    std::string DisplayLUTAsString(vtkLookupTable *lut);

    //! Compare two lookup tables.
    /*!
     * @param lut1 - the lookup table.
     * @param lut2 - the lookup table.
     * @return true if the tables are the same.
     */
    std::pair<bool, std::string> CompareLUTs(vtkLookupTable *lut1, vtkLookupTable *lut2);

  private:
    //! Get a string of [R, G, B, A] as double.
    std::string RGBAToDoubleString(double *rgba);

    //! Get a string of [R, G, B, A] as unsigned char.
    std::string RGBAToCharString(double *rgba);

    //! Get a hexadecimal string of the RGB colors.
    std::string RGBToHexString(double *rgba);

    //! Get a string of [R, G, B, A] as double, unsigned char and hex.
    std::string AssembleRGBAString(double *rgba);

    //! Compare two rgba colors.
    template <typename T> bool CompareRGBA(T *rgba1, T *rgba2);
};

