#include "mpi/mpi_manager.hpp"
#ifdef USING_MPI
namespace icr =  institute_of_cancer_research;


icr::mpi_command_base::mpi_command_base(size_t job_id) 
   :  
  m_id(job_id+ MPI_COUNT_OFFSET), 
  m_empty(false), 
  m_replied(false) 
{}

icr::mpi_command_base::~mpi_command_base()
{}

void
icr::mpi_command_base::set_id(size_t id)
{
  m_id = id + MPI_COUNT_OFFSET;
}


size_t 
icr::mpi_command_base::id() const
{
  return m_id;
}

size_t 
icr::mpi_command_base::get_id() const
{
  return id();
}

bool 
icr::mpi_command_base::empty() const
{
  return m_empty;
}

void 
icr::mpi_command_base::set_empty(bool is_empty )
{
  m_empty = is_empty;
}








#endif
