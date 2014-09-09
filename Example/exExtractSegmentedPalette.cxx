/*=========================================================================
                                                                                
  Program:   gdcm
  Module:    $RCSfile: exExtractSegmentedPalette.cxx,v $
  Language:  C++
  Date:      $Date: 2007/10/03 08:50:48 $
  Version:   $Revision: 1.2 $
                                                                                
  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
  l'Image). All rights reserved. See Doc/License.txt or
  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
                                                                                
     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.
                                                                                
=========================================================================*/

#include <gdcm.h>
// Ref: 
// http://blog.goo.ne.jp/satomi_takeo/e/3643e5249b2a9650f9e10ef1c830e8b8
// I bet the code was compiled on VS6. Make it compile on other platform:
// * typedef are not inherited
// * need to explicitely add typename keyword
// * Uint8 / Uint16 are neither C nor C++
// * replace all dcmtk code with identical gdcm code

#include <assert.h>
#include <algorithm>
#include <deque>
#include <map>
#include <vector>
#include <iterator>

namespace {
    // abstract class for segment.
    template <typename EntryType>
    class Segment {
    public:
        typedef std::map<const EntryType*, const Segment*> SegmentMap;
        virtual bool Expand(const SegmentMap& instances,
            std::vector<EntryType>& expanded) const = 0;
        const EntryType* First() const { return _first; }
        const EntryType* Last() const { return _last; }
        struct ToMap {
            std::pair<
                typename SegmentMap::key_type,
                typename SegmentMap::mapped_type
            >
                operator()(const Segment* segment) const
            { return std::make_pair(segment->First(), segment); }
        };
    protected:
        Segment(const EntryType* first, const EntryType* last) {
            _first = first; _last = last;
        }
        const EntryType* _first;
        const EntryType* _last;
    };

    // discrete segment (opcode = 0)
    template <typename EntryType>
    class DiscreteSegment : public Segment<EntryType> {
    public:
        typedef typename Segment<EntryType>::SegmentMap SegmentMap;
        DiscreteSegment(const EntryType* first)
            : Segment<EntryType>(first, first+2+*(first+1)) {}
        virtual bool Expand(const SegmentMap&,
            std::vector<EntryType>& expanded) const
        {
            std::copy(this->_first + 2, this->_last, std::back_inserter(expanded));
            return true;
        }
    };

    // linear segment (opcode = 1)
    template <typename EntryType>
    class LinearSegment : public Segment<EntryType> {
    public:
        typedef typename Segment<EntryType>::SegmentMap SegmentMap;
        LinearSegment(const EntryType* first)
            : Segment<EntryType>(first, first+3) {}
        virtual bool Expand(const SegmentMap&,
            std::vector<EntryType>& expanded) const
        {
            if ( expanded.empty() ) {
                // linear segment can't be the first segment.
                return false;
            }
            EntryType length = *(this->_first + 1);
            EntryType y0 = expanded.back();
            EntryType y1 = *(this->_first + 2);
            double y01 = y1 - y0;
            for ( EntryType i = 0; i <length; ++i ) {
                double value_float
                    = static_cast<double>(y0)
                    + (static_cast<double>(i)/static_cast<double>(length)) * y01;
                EntryType value_int = static_cast<EntryType>(value_float + 0.5);
                expanded.push_back(value_int);
            }
            return true;
        }
    };

    // indirect segment (opcode = 2)
    template <typename EntryType>
    class IndirectSegment : public Segment<EntryType> {
    public:
        typedef typename Segment<EntryType>::SegmentMap SegmentMap;
        IndirectSegment(const EntryType* first)
            : Segment<EntryType>(first, first+2+4/sizeof(EntryType)) {}
        virtual bool Expand(const SegmentMap& instances,
            std::vector<EntryType>& expanded) const
        {
            if ( instances.empty() ) {
                // some other segments are required as references.
                return false;
            }
            const EntryType* first_segment = instances.begin()->first;
            const unsigned short* pOffset
                = reinterpret_cast<const unsigned short*>(this->_first + 2);
            unsigned long offsetBytes
                = (*pOffset) | (static_cast<unsigned long>(*(pOffset + 1)) << 16);
            const EntryType* copied_part_head
                = first_segment + offsetBytes / sizeof(EntryType);
            typename SegmentMap::const_iterator ppHeadSeg = instances.find(copied_part_head);
            if ( ppHeadSeg == instances.end() ) {
                // referred segment not found
                return false;
            }
            EntryType nNumCopies = *(this->_first + 1);
            typename SegmentMap::const_iterator ppSeg = ppHeadSeg;
            while ( std::distance(ppHeadSeg, ppSeg) <nNumCopies ) {
                assert( ppSeg != instances.end() );
                ppSeg->second->Expand(instances, expanded);
                ++ppSeg;
            }
            return true;
        }
    };

    template <typename EntryType>
    void ExpandPalette(const EntryType* raw_values, uint32_t length,
        std::vector<EntryType>& palette)
    {
        typedef std::deque<Segment<EntryType>*> SegmentList;
        SegmentList segments;
        const EntryType* raw_seg = raw_values;
        while ( (std::distance(raw_values, raw_seg) * sizeof(EntryType)) <length ) {
            Segment<EntryType>* segment = NULL;
            if ( *raw_seg == 0 ) {
                segment = new DiscreteSegment<EntryType>(raw_seg);
            } else if ( *raw_seg == 1 ) {
                segment = new LinearSegment<EntryType>(raw_seg);
            } else if ( *raw_seg == 2 ) {
                segment = new IndirectSegment<EntryType>(raw_seg);
            }
            if ( segment ) {
                segments.push_back(segment);
                raw_seg = segment->Last();
            } else {
                // invalid opcode
                break;
            }
        }
        typename Segment<EntryType>::SegmentMap instances;
        std::transform(segments.begin(), segments.end(),
            std::inserter(instances, instances.end()), typename Segment<EntryType>::ToMap());
        typename SegmentList::iterator ppSeg = segments.begin();
        typename SegmentList::iterator endOfSegments = segments.end();
        for ( ; ppSeg != endOfSegments; ++ppSeg ) {
            (*ppSeg)->Expand(instances, palette);
        }
        ppSeg = segments.begin();
        for ( ; ppSeg != endOfSegments; ++ppSeg ) {
            delete *ppSeg;
        }
    }

    void ReadPalette(GDCM_NAME_SPACE::File* pds, const GDCM_NAME_SPACE::TagKey& descriptor,
      const GDCM_NAME_SPACE::TagKey& segment)
      {
      int desc_values[3] = {};
      unsigned long count = 0;
      //if ( pds->findAndGetUint16Array(descriptor, desc_values, &count).good() )
      std::string desc_values_str = pds->GetEntryString(descriptor.GetGroup(), descriptor.GetElement() );
      count = sscanf( desc_values_str.c_str(), "%d\\%d\\%d", desc_values, desc_values+1, desc_values+2 );
        {
        assert( count == 3 );
        unsigned int num_entries = desc_values[0];
        if ( num_entries == 0 ) {
          num_entries = 0x10000;
        }
        int min_pixel_value = desc_values[1];
        unsigned int entry_size = desc_values[2];
        assert( entry_size == 8 || entry_size == 16 );
        //DcmElement* pe = NULL;
        GDCM_NAME_SPACE::DataEntry* pe = NULL;

        pe = pds->GetDataEntry(segment.GetGroup(), segment.GetElement() ); {
          //if ( pds->findAndGetElement(segment, pe).good() )
          unsigned long length = pe->GetLength();
          if ( entry_size == 8 ) {
            uint8_t* segment_values = NULL;
            //if ( pe->getUint8Array(segment_values).good() )
            segment_values = (uint8_t*)pe->GetBinArea(); {
              std::vector<uint8_t> palette;
              palette.reserve(num_entries);
              ExpandPalette(segment_values, length, palette);
            }
          } else if ( entry_size == 16 ) {
            uint16_t* segment_values = NULL;
            segment_values = (uint16_t*)pe->GetBinArea(); {
              //if ( pe->getUint16Array(segment_values).good() )
              std::vector<uint16_t> palette;
              palette.reserve(num_entries);
              ExpandPalette(segment_values, length, palette);
              std::copy(palette.begin(), palette.end(), 
                std::ostream_iterator<uint16_t>(std::cout, "\n"));

            }
          }
        }
        }
      }
} // end namespace

int main(int argc, char* argv[])
{
//    if ( argc < 2 ) {
//        return 1;
//    }
//    DcmFileFormat ff;
//    OFString filename(argv[1]);
//    if ( ff.loadFile(filename.c_str()).good() ) {
//        DcmDataset* pDataset = ff.getDataset();
//        assert( pDataset );
//        OFString pi;
//        if ( pDataset->findAndGetOFString(DCM_PhotometricInterpretation, pi).good()
//            && pi == OFString("PALETTE COLOR") ) {
//            ReadPalette(pDataset, DCM_RedPaletteColorLookupTableDescriptor,
//                DCM_SegmentedRedPaletteColorLookupTableData);
//            ReadPalette(pDataset, DCM_GreenPaletteColorLookupTableDescriptor,
//                DCM_SegmentedGreenPaletteColorLookupTableData);
//            ReadPalette(pDataset, DCM_BluePaletteColorLookupTableDescriptor,
//                DCM_SegmentedBluePaletteColorLookupTableData);
//        }
//    }

  GDCM_NAME_SPACE::File *f = GDCM_NAME_SPACE::File::New();
  f->SetFileName( argv[1] );
  bool res = f->Load();
  f->Print( std::cout );
  GDCM_NAME_SPACE::TagKey DCM_RedPaletteColorLookupTableDescriptor (0x0028, 0x1101);
  GDCM_NAME_SPACE::TagKey DCM_GreenPaletteColorLookupTableDescriptor (0x0028, 0x1102);
  GDCM_NAME_SPACE::TagKey DCM_BluePaletteColorLookupTableDescriptor (0x0028, 0x1103);

  GDCM_NAME_SPACE::TagKey DCM_SegmentedRedPaletteColorLookupTableData (0x0028, 0x1221);
  GDCM_NAME_SPACE::TagKey DCM_SegmentedGreenPaletteColorLookupTableData (0x0028, 0x1222);
  GDCM_NAME_SPACE::TagKey DCM_SegmentedBluePaletteColorLookupTableData (0x0028, 0x1223);


  // TODO need to check file is indeed PALETTE COLOR:
  ReadPalette(f, DCM_RedPaletteColorLookupTableDescriptor,
    DCM_SegmentedRedPaletteColorLookupTableData);
  ReadPalette(f, DCM_GreenPaletteColorLookupTableDescriptor,
    DCM_SegmentedGreenPaletteColorLookupTableData);
  ReadPalette(f, DCM_BluePaletteColorLookupTableDescriptor,
    DCM_SegmentedBluePaletteColorLookupTableData);

    return 0;
}

