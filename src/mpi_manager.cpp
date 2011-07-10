
/***********************************************************************************
 ***********************************************************************************
 **                                                                               **
 **  Copyright (C) 2011 Tom Shorrock <t.h.shorrock@gmail.com> 
 **                                                                               **
 **                                                                               **
 **  This program is free software; you can redistribute it and/or                **
 **  modify it under the terms of the GNU General Public License                  **
 **  as published by the Free Software Foundation; either version 2               **
 **  of the License, or (at your option) any later version.                       **
 **                                                                               **
 **  This program is distributed in the hope that it will be useful,              **
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of               **
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                **
 **  GNU General Public License for more details.                                 **
 **                                                                               **
 **  You should have received a copy of the GNU General Public License            **
 **  along with this program; if not, write to the Free Software                  **
 **  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  **
 **                                                                               **
 ***********************************************************************************
 ***********************************************************************************/



#include "mpi_manager/mpi_manager.hpp"



ICR::mpi_command_base::mpi_command_base(size_t job_id) 
   :  
  m_id(job_id+ MPI_COUNT_OFFSET), 
  m_empty(false), 
  m_replied(false) 
{}

ICR::mpi_command_base::~mpi_command_base()
{
}

void
ICR::mpi_command_base::set_id(size_t id)
{
  m_id = id + MPI_COUNT_OFFSET;
}


size_t 
ICR::mpi_command_base::id() const
{
  return m_id;
}

size_t 
ICR::mpi_command_base::get_id() const
{
  return id();
}

bool 
ICR::mpi_command_base::empty() const
{
  return m_empty;
}

void 
ICR::mpi_command_base::set_empty(bool is_empty )
{
  m_empty = is_empty;
}



