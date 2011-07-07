
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



#pragma once
#ifndef MPI_MANAGER_HPP
#define MPI_MANAGER_HPP



#ifdef USING_MPI


#include <sstream>

#include <boost/mpi.hpp>
#include <iostream>
#include <boost/serialization/string.hpp>

#include <queue>


namespace mpi = boost::mpi;
using std::deque;

//! The main Institute of Cancer Research namespace
namespace ICR{

  enum {
    MPI_REQUEST_JOB,
    MPI_RECIEVE_JOB,
    MPI_JOB
  };
  

  class mpi_command_base{
    //make it serialisable
    friend class boost::serialization::access;
    template<class Archive>  void serialize(Archive & ar, const unsigned int version)
    {    ar & m_id;    ar & m_empty;    ar & m_replied;  }

    size_t m_id;
    bool m_empty; //if sends no data;
    bool m_replied; 
    //mpi::environment& m_env;
    //mpi::communicator m_world;
  public:
    mpi_command_base(size_t job_id = 0 , bool empty = true) 
      :  m_id(job_id), m_empty(empty), m_replied(false) {};
    virtual ~mpi_command_base(){ 
      //reply();
    };
    //mpi_command<T>& void set_task(T& task) {m_job = task; m_empty=false; };
    //void set_task (mpi_command_base& other){ *this = other; };
    void set_id(size_t id){m_id = id;};
    size_t get_id(){return m_id;};
    bool empty(){return m_empty;}; 
    virtual void run() = 0;
    void reply(mpi::communicator& world) {
      if (m_replied ==false){
	world.isend(0, MPI_JOB, *this );
	m_replied = true;
      }
    };

  };




  class 
  mpi_request_job{
    //make it serialisable
    friend class boost::serialization::access;
    template<class Archive>  void serialize(Archive & ar, const unsigned int version)
    {    ar & m_id;   }
  
    size_t m_id;
    //      mpi::environment env;
    //mpi::communicator world;
  public:
     mpi_request_job(mpi::communicator& world) : m_id( world.rank() ) { world.isend(0, MPI_REQUEST_JOB, *this ); };
    size_t id() const {return m_id;};
  };


  
  template <class command >
  class mpi_manager
  {
  private:
    deque< command > m_inbox;
    deque< command > m_outbox;
  

  public:
    mpi_manager(){};
    mpi_manager(deque< command >& inbox, mpi::communicator& world   );
    void set_inbox(deque< command >& inbox) {m_inbox = inbox;};
    deque< command > get_outbox(){return m_outbox;};
  };


  
  template<class command>
  mpi_manager<command>::mpi_manager(deque< command >& inbox, mpi::communicator& world   )
    : m_inbox(inbox)
  {
    
    // mpi::communicator world;
    
    //if server
    if (world.rank() == 0)
      {
	//set-up reply list and count
 	deque<mpi::request> replied ;
 	size_t count = 0;
 	//	size_t open_nodes = world.size() - 1; //-1 because server isn't a node
	while (count < m_inbox.size() + m_outbox.size() + world.size() ) {
 	  //total size + a handshake
 	  //wait for request
	  mpi::status request = world.probe(mpi::any_source, MPI_REQUEST_JOB);
 	  //mpi_request_job request(world);
	  //  world.irecv(1, MPI_RECIEVE_JOB , request);
	  //	  world.recv(1,0,request); 
 	  //create a job;
 	  command job(count);
 	  if (m_inbox.size() !=0){
 	    job = m_inbox.front() ;
 	    m_inbox.pop_front();
	  }
	  // 	  else //nothing to process, send an empty job
	  //  job.set_id(count);
	  
 	  //send job	  
 	  world.isend(request.source(), count, job);
	  
 	  //listen for reply
 	  m_outbox.push_back( job ); //the original job will be altered
 	  replied[count] = world.irecv(request.source(), count, m_outbox.back() );
 	  ++count;
 	};
// 	mpi::wait_all(replied.begin(), replied.end());
      }
    else   //if client
      {
	bool still_data = true;
	while (still_data){
	  //request      
	  
	  //mpi_request_job job_please(world);
	  world.send(0, MPI_REQUEST_JOB);
	  //wait for job
	  command job;
	  world.recv(0, MPI_REQUEST_JOB , job);
	  //check to see if there is any data;
	  if (job.empty() ) {still_data = false;}
	  else {
	    //we are good to go.
	    job.run();
	  }
	  //return data
	  job.reply(world);
	};
    
      }


  }

}


#endif  //gaurd for using MPI

#endif  // guard for MPI_MANAGER_HPP
