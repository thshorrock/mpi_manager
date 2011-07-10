
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
#ifndef MPI_MANAGER_MPI_MANAGER_HPP
#define MPI_MANAGER_MPI_MANAGER_HPP


#include <boost/progress.hpp>
#include <boost/timer.hpp>
#include <sstream>

#include <boost/mpi.hpp>
#include <iostream>
#include <boost/serialization/string.hpp>
#include <boost/serialization/base_object.hpp>
#include <queue>


namespace mpi = boost::mpi;
using std::deque;

//! The main Institute of Cancer Research namespace
namespace ICR{
  
  //! Constants that label the messages between the processors
  //negatives seem to upset things
  const int MPI_REQUEST_PREDATA = 1;
  const int MPI_RECIEVE_PREDATA = 2;
  const int MPI_REQUEST_JOB = 3;
  const int MPI_RECIEVE_JOB = 4;
  const int MPI_JOB = 5;
  const int MPI_JOBS_END = 6;
  const int MPI_HANDSHAKE = 7;
  const int MPI_COUNT_OFFSET = 10;
  

  class mpi_command_base{

    size_t m_id;
    bool m_empty; //if sends no data;
    bool m_replied;
    //make it serialisable
    friend class boost::serialization::access;
    template<class Archive>  void serialize(Archive & ar, const unsigned int version)
    {    ar & m_id;    ar & m_empty;    ar & m_replied;  }

  public:
    mpi_command_base(size_t job_id = 0);
    
    virtual 
    ~mpi_command_base();
    
    void 
    set_id(size_t id);
    
    size_t 
    id() const;
    
    size_t 
    get_id() const;
    
    bool 
    empty() const;
    
    void 
    set_empty(bool is_empty = true);

    virtual void run() = 0;
  };

    
  template <class command >
  class mpi_manager
  {
  private:
    deque< command > m_inbox;
    deque< command > m_outbox;
    bool m_display;
    bool m_verbose;
    mpi::communicator m_world;
    //void run();

  public:
    mpi_manager(){};
    mpi_manager(const mpi::communicator& world, const deque< command >& inbox, bool display = false, bool verbose = false);
    void operator()();
    void run(){return operator()();};
    
    ~mpi_manager(){
      //make sure all the processes finish at the same time.
      m_world.barrier();
    };
    deque< command > get_outbox(){return m_outbox;};
  };



}

template<class command>
ICR::mpi_manager<command>::mpi_manager(const mpi::communicator& world, const deque< command >& inbox, bool display, bool verbose  )
  : m_inbox(inbox), m_display(display), m_verbose(verbose), m_world(world)
{}


template<class command>
void
ICR::mpi_manager<command>::operator()()
{

 
  //where the asyncrinous messages are stored on each node (so can check to see if they have arrived)
  deque<mpi::request> assync_messages ;
  if (m_world.rank() == 0)   //if server
    {
	
      //set-up reply list and count
      size_t count = 0;  //the number of items in the inbox that has been completed
      size_t max_id = 0;
      //the total number of messages to be sent is the size of the inbox  plusa handshake to each of the nodes (-1 because server isn't a node)
      const size_t no_messages = m_inbox.size() + (m_world.size() -1) ;	
      boost::progress_display* pd;
      if (m_display && !m_verbose) {pd = new boost::progress_display(no_messages); }

      while (count < no_messages ) { 
        //wait for request   //
        
        if(m_verbose)  std::cout<<"SERVER: waiting for request"<<std::endl;
        mpi::status request = m_world.probe(mpi::any_source, MPI_REQUEST_JOB);  //find out where request comes from
       
        if(m_verbose) std::cout<<"SERVER: request recieved from "<<request.source()<<std::endl;
	  
        //create a job;
        command job;
        if (m_inbox.size() !=0){  //there is a job to send
          job = m_inbox.front() ;  //copy the job from the inbox
	  job.set_id(count);  //set the id
          m_inbox.pop_front();     //pop the inbox
          //send job	    //
	  if(m_verbose)  std::cout<<"SERVER: sending job to "<<request.source()<<std::endl;	  
          m_world.send(request.source(),MPI_RECIEVE_JOB, job);
          //create the outbox job
          m_outbox.push_back( job ); //the original job will be altered
          //listen for reply and recieve completed jobs
          assync_messages.push_back( m_world.irecv(request.source(),job.id() , m_outbox.back() ) );
          //update max_id
          if (job.id() > max_id) {max_id = job.id();}
        }
        else{  //there is no job to send  //
          if(m_verbose) std::cout<<"SERVER: out of jobs!"<<std::endl;
          //flag the job as empty and send
          job.set_empty();
          job.set_id(++max_id); //make sure id is unique
          m_world.send(request.source(),MPI_RECIEVE_JOB, job);
          //wait for acknowledgement that job is finished (the handshake)
	  assync_messages.push_back( m_world.irecv(request.source(),MPI_HANDSHAKE ));
        }
        ++count;  
        if (m_display && !m_verbose) ++(*pd);
        //close the original job request, the job is done
        m_world.recv(request.source(),MPI_REQUEST_JOB);
      };
      if (m_display && !m_verbose) delete pd;
      if(m_verbose)  
	std::cout<<"SERVER: Waiting for everything to finish ("<< assync_messages.end()-assync_messages.begin() <<" messages)"<<std::endl;
      mpi::wait_all(assync_messages.begin(), assync_messages.end());
      if(m_verbose) 
	std::cout<<"SERVER: Everything is done"<<std::endl;
    }
  else //   if client
    {
      bool still_data = true;  //there are jobs to be done
      while (still_data) { 
        //request a job from server
        m_world.send(0,MPI_REQUEST_JOB );
        //create a job and wait until it is recieved // 
        if(m_verbose)  std::cout<<"CLIENT "<< m_world.rank()<<": waiting for a job"<<std::endl;
        command job;
        m_world.recv(0, MPI_RECIEVE_JOB , job);
        //check to see if there is any data;
        if (job.empty() ) { // 
          if(m_verbose)  std::cout<<"CLIENT "<< m_world.rank()<<": job recieved is empty"<<std::endl;
          still_data = false;
	  //wait for previous (completed) jobs to be recieved by server
	  mpi::wait_all(assync_messages.begin(), assync_messages.end());
	  //handshake 
	  mpi::request handshake = m_world.isend(0, MPI_HANDSHAKE );
	  handshake.wait();
	  if(m_verbose) std::cout<<"HANDSHAKE COMPLETE FOR "<<m_world.rank()<<std::endl;
	}
        else {
          //we are good to go.  //  
          if(m_verbose) std::cout<<"CLIENT "<< m_world.rank()<<": job has data"<<std::endl;
          job.run();
	  assync_messages.push_back(m_world.isend(0, job.id(), job ));
        }
      };
    }

  if(m_verbose) std::cout<<"ALL: Complete for "<<m_world.rank()<<std::endl;
}

BOOST_IS_MPI_DATATYPE(ICR::mpi_command_base)
//BOOST_IS_MPI_DATATYPE(mpi_manager)
BOOST_CLASS_TRACKING(ICR::mpi_command_base,track_never)
//BOOST_CLASS_IMPLEMENTATION(mpi_command_base,object_serializable)
//BOOST_CLASS_TRACKING(mpi_manager,track_never)
//BOOST_CLASS_IMPLEMENTATION(mpi_manager,object_serializable)

/* IMPLEMENTATION */


// inline
// icr::mpi_command_base::~mpi_command_base(){
//   if (!m_replied){
//     //std::cout<<"warning: need to reply"<<std::endl;

//     //throw "need to reply";
//     //need to reply, create a m_world to do so.
//     //This is a bit expensive so its best to use the reply option with a preconstructed m_world if one exists.
//     //mpi::communicator m_world;
    
//     //  reply(m_world);
//   }
// }


// inline  void
// icr::mpi_command_base::reply(mpi::communicator& m_world, mpi::request& r) {
//   if (m_replied ==false){
//     r = m_world.isend(0, id(), *this );
//     m_replied = true;
//     }
// }
    


#endif  // guard for MPI_MANAGER_MPI_MANAGER_HPP
