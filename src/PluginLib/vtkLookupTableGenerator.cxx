#include "vtkLookupTableGenerator.h"

//! Display the contents of the lookup table.
/*!
 * @param lut - the lookup table.
 * @return a string containing the table data.
 */
std::string vtkLookupTableGenerator::DisplayLUTAsString(vtkLookupTable *lut)
{
  vtkIdType tv = lut->GetNumberOfTableValues();
  double dR[2];
  lut->GetTableRange(dR);
  std::ostringstream os;
  if (lut->GetIndexedLookup())
    {
    vtkIdType av = lut->GetNumberOfAnnotatedValues();
    os << "Categorical Lookup Table\nNumber of annotated values: " << av
      << " Number of table values: " << tv
      << "\nTable Range: " << std::fixed
       << std::setw(8) << std::setprecision(6)
      << dR[0] << " to " << dR[1] << std::endl;
    if (av > 0)
      {
      for (vtkIdType i = 0; i < av; ++i)
        {
        double rgba[4];
        lut->GetAnnotationColor(lut->GetAnnotatedValue(i), rgba);
        os << std::setw(5) << lut->GetAnnotation(i) << ": ";
        os << this->AssembleRGBAString(rgba);
        os << std::endl;
        }
      }
    else
      {
      for (vtkIdType i = 0; i < tv; ++i)
        {
        double rgba[4];
        lut->GetTableValue(i, rgba);
        os << std::setw(5) << i << ": ";
        os << this->AssembleRGBAString(rgba);
        os << std::endl;
        }
      }
    }
  else
    {
    os << "Ordinal Lookup Table\nNumber of table values : " << tv
      << "\nTable Range: " << std::fixed
       << std::setw(8) << std::setprecision(6)
      << dR[0] << " to " << dR[1] << std::endl;
    std::vector<double> indices;
    for (int i = 0; i < tv; ++i)
      {
      indices.push_back((dR[1] - dR[0]) * i / tv + dR[0]);
      }
    for (std::vector<double>::const_iterator p = indices.begin();
        p != indices.end(); ++p)
      {
      double rgba[4];
      lut->GetColor(*p, rgba);
      rgba[3] = lut->GetOpacity(*p);
      os << std::fixed << std::setw(5) << std::setprecision(2)
         << *p << ": ";
      os << this->AssembleRGBAString(rgba);
      os << std::endl;
      }
    }
  return os.str();
}

//-----------------------------------------------------------------------------
//! Compare two lookup tables.
/*!
 * @param lut1 - the lookup table.
 * @param lut2 - the lookup table.
 * @return true if the tables are the same.
 */
std::pair<bool, std::string> vtkLookupTableGenerator::CompareLUTs(vtkLookupTable *lut1,
vtkLookupTable *lut2)
{
  std::pair<bool, std::string> res(true, "");
  if (lut1->GetIndexedLookup() != lut2->GetIndexedLookup())
    {
    res.first = false;
    res.second = "One table is ordinal and the other is categorical.";
    return res;
    }
  if (lut1->GetIndexedLookup() &&
      lut1->GetNumberOfAnnotatedValues()
      != lut2->GetNumberOfAnnotatedValues())
    {
    res.first = false;
    res.second = "The number of annotated values do not match.";
    return res;
    }
  if (lut1->GetNumberOfTableValues() != lut2->GetNumberOfTableValues())
    {
    res.first = false;
    res.second = "Table values do not match.";
    return res;
    }
  double dR1[2];
  lut1->GetTableRange(dR1);
  double dR2[2];
  lut2->GetTableRange(dR2);
  if (dR1[0] != dR2[0] && dR2[1] != dR1[1])
    {
    res.first = false;
    res.second = "Table ranges do not match.";
    }
  if (lut1->GetIndexedLookup())
    {
    vtkIdType av = lut1->GetNumberOfAnnotatedValues();
    if (av > 0)
      {
      for (vtkIdType i = 0; i < av; ++i)
        {
        if (lut1->GetAnnotation(i) != lut1->GetAnnotation(i))
          {
          res.first = false;
          res.second = "Annotations do not match.";
          return res;
          }
        }
      for (vtkIdType i = 0; i < av; ++i)
        {
        double rgba1[4];
        lut1->GetAnnotationColor(lut1->GetAnnotatedValue(i), rgba1);
        double rgba2[4];
        lut2->GetAnnotationColor(lut2->GetAnnotatedValue(i), rgba2);
        if (!this->CompareRGBA(rgba1, rgba2))
          {
          res.first = false;
          res.second = "Colors do not match.";
          return res;
          }
        }
      }
    else
      {
      for (vtkIdType i = 0; i < av; ++i)
        {
        double rgba1[4];
        lut1->GetTableValue(i, rgba1);
        double rgba2[4];
        lut2->GetTableValue(i, rgba2);
        if (!this->CompareRGBA(rgba1, rgba2))
          {
          res.first = false;
          res.second = "Colors do not match.";
          return res;
          }
        }
      }
    }
  else
    {
    vtkIdType tv = lut1->GetNumberOfTableValues();
    std::vector<double> indices;
    for (int i = 0; i < tv; ++i)
      {
      indices.push_back((dR1[1] - dR1[0]) * i / tv + dR1[0]);
      }
    for (std::vector<double>::const_iterator p = indices.begin();
      p != indices.end(); ++p)
      {
      double rgba1[4];
      lut1->GetColor(*p, rgba1);
      rgba1[3] = lut1->GetOpacity(*p);
      double rgba2[4];
      lut2->GetColor(*p, rgba2);
      rgba2[3] = lut2->GetOpacity(*p);
      if (!this->CompareRGBA(rgba1, rgba2))
        {
        res.first = false;
        res.second = "Colors do not match.";
        return res;
        }
      }
    }
  return res;
}

//-----------------------------------------------------------------------------
//! Get a string of [R, G, B, A] as double.
std::string vtkLookupTableGenerator::RGBAToDoubleString(double *rgba)
{
  std::ostringstream os;
  os << "[";
  for (int i = 0; i < 4; ++i)
    {
    if (i == 0)
      {
      os << std::fixed << std::setw(8) << std::setprecision(6) << rgba[i];
      }
    else
      {
      os << std::fixed << std::setw(9) << std::setprecision(6) << rgba[i];
      }
    if (i < 3)
      {
      os << ",";
      }
    }
  os << "]";
  return os.str();
}

//-----------------------------------------------------------------------------
//! Get a string of [R, G, B, A] as unsigned char.
std::string vtkLookupTableGenerator::RGBAToCharString(double *rgba)
{
  std::ostringstream os;
  os << "[";
  for (int i = 0; i < 4; ++i)
    {
    if (i == 0)
      {
      os << std::setw(3) << static_cast<int>(rgba[i] * 255);
      }
    else
      {
      os << std::setw(4) << static_cast<int>(rgba[i] * 255);
      }
    if (i < 3)
      {
      os << ",";
      }
    }
  os << "]";
  return os.str();
}

//-----------------------------------------------------------------------------
//! Get a hexadecimal string of the RGB colors.
std::string vtkLookupTableGenerator::RGBToHexString(double *rgba)
{
  std::ostringstream os;
  for (int i = 0; i < 3; ++i)
    {
    os << std::setw(2) << std::setfill('0') << std::hex
       << static_cast<int>(rgba[i] * 255);
    }
  return os.str();
}

//-----------------------------------------------------------------------------
//! Get a string of [R, G, B, A] as double, unsigned char and hex.
std::string vtkLookupTableGenerator::AssembleRGBAString(double *rgba)
{
  std::ostringstream os;
  os << this->RGBAToDoubleString(rgba);
  os << " ";
  os << this->RGBAToCharString(rgba);
  os << " 0x";
  os << this->RGBToHexString(rgba);
  return os.str();
}

//-----------------------------------------------------------------------------
//! Compare two rgba colors.
template <typename T>
bool vtkLookupTableGenerator::CompareRGBA(T *rgba1, T *rgba2)
{
  bool areEquivalent = true;
  for (int i = 0; i < 4; ++i)
    {
    areEquivalent &= rgba1[i] == rgba2[i];
    if (!areEquivalent)
      {
      return false;
      }
    }
  return true;
}


vtkSmartPointer<vtkLookupTable> vtkLookupTableGenerator::GenerateLUT(int idx){
    vtkSmartPointer<vtkColorSeries> colorSeries = vtkSmartPointer<vtkColorSeries>::New();

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    colorSeries->SetColorScheme(idx);
    colorSeries->BuildLookupTable(lut);

    return lut;
}


