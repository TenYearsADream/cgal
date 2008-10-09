// Copyright (c) 2004  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
// Author(s)     : Laurent Saboret
//                 (based on Memory_sizer.h by Sylvain Pion and Andreas Fabri)

#ifndef CGAL_PEAK_MEMORY_SIZER_H
#define CGAL_PEAK_MEMORY_SIZER_H

#include <CGAL/Memory_sizer.h>
#include <CGAL/Taucs_fix.h>
#include <CGAL/surface_reconstruction_assertions.h>

#include <cmath>
#include <cfloat>
#include <climits>

CGAL_BEGIN_NAMESPACE


/// Peak_memory_sizer extends Memory_sizer with new memory statistics.
struct Peak_memory_sizer : public Memory_sizer
{
    typedef std::size_t   size_type;

    /// Get the peak memory used by the process.
    /// Both the virtual memory size and the resident size.
    size_type peak_virtual_size()  const { return get_peak_memory(true); }
    size_type peak_resident_size() const { return get_peak_memory(false); }

    /// Get the number of large free memory blocks.
    size_type count_free_memory_blocks(size_type min_block_size) const
    {
        // Allocate all memory blocks >= min_block_size
        std::vector<void*> blocks;
        void* block;
        while ((block = malloc(min_block_size)) != NULL)
          blocks.push_back(block);

        // Return value
        size_type count = blocks.size();

        // Free large memory blocks
        for (size_type i=0; i<count; i++)
          free(blocks[i]);

        return count;
    }

    /// Give size of largest block available for allocation
    size_t largest_free_block() const
    {
      size_t m_sys;
      size_t m,m_low,m_high,m_tol;
      char*  p;
      size_t m_max;

      // Get physical system memory size
      m_sys = (size_t) (taucs_system_memory_size() + 0.5);

      // If m_sys is meaningful, then we limit malloc test by m_sys
      // to avoid an infinite loop on linux (malloc() never returns null
      // due to "optimistic memory allocation").
      // TODO: replace m_sys by virtual address space limit?        
      if (m_sys > 0)
        m_max = m_sys;
      else
        m_max = (std::numeric_limits<size_t>::max)();

      /* malloc test */

      m = 1048576;

      while ( (m < m_max-1) /* m_max not reached */
           && ((p=(char*) malloc( (std::min)(m_max,m*2) )) != NULL) ) {
        //CGAL_TRACE("largest_free_block: %.2lf Mb\n", double((std::min)(m_max,m*2)) / 1048576.0);
        free(p);
        m = (std::min)(m_max,m*2);
      }

      m_low  = m;
      m_high = (std::min)(m_max,m*2);
      m_tol  = m / 128;

      while ( m_high - m_low > m_tol ) {
        m = m_low + ( (m_high-m_low)/2 );
        //CGAL_TRACE("largest_free_block: [%.2lf %.2lf %.2lf]\n",
	//           (double)m_low  / 1048576.0,
	//           (double)m      / 1048576.0,
	//           (double)m_high / 1048576.0);
        if ( (p=(char*) malloc(m)) != NULL )
          m_low = m;
        else
          m_high = m;
        free(p);
      }

      m = m_low;

      //CGAL_TRACE("largest_free_block: malloc test=%.2lf MB sys test=%.2lf MB\n",
      //      (double)m      / 1048576.0,
      //      (double)m_sys  / 1048576.0);

      return m;
    }

private:

  size_type get_peak_memory (bool virtual_size)  const
  {
#ifdef _MSC_VER
    DWORD pid = GetCurrentProcessId();
    size_type result;
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;
    hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
                                    PROCESS_VM_READ,
                                    FALSE, pid );
    if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
    {
//CGAL_TRACE("    Peak_memory_sizer: WorkingSetSize=%ld Mb\n",              pmc.WorkingSetSize>>20);
//CGAL_TRACE("    Peak_memory_sizer: PagefileUsage=%ld Mb\n",               pmc.PagefileUsage>>20);
//CGAL_TRACE("    Peak_memory_sizer: PeakWorkingSetSize=%ld Mb\n",          pmc.PeakWorkingSetSize>>20);
//CGAL_TRACE("    Peak_memory_sizer: PeakPagefileUsage=%ld Mb\n",           pmc.PeakPagefileUsage>>20);

      // LS 10/2008: PeakPagefileUsage seems unreliable, thus we use an approximation:
      size_t memory_paged_out = (pmc.PagefileUsage>pmc.WorkingSetSize) ? (pmc.PagefileUsage-pmc.WorkingSetSize) : 0;
      size_t approximate_peak_virtual_size = pmc.PeakWorkingSetSize + memory_paged_out;
//CGAL_TRACE("    Peak_memory_sizer: approximate_peak_virtual_size=%ld Mb\n", approximate_peak_virtual_size>>20);

      result = virtual_size ? approximate_peak_virtual_size : pmc.PeakWorkingSetSize;
    }

    CloseHandle( hProcess );
    return result;

#else
    // Not yet implemented
    return 0;
#endif
  }
};


CGAL_END_NAMESPACE

#endif // CGAL_PEAK_MEMORY_SIZER_H
