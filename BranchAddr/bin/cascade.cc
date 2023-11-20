#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Candidate/interface/Candidate.h"

#include "SimDataFormats/Track/interface/SimTrack.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"


#include "TClass.h"
// #include "TCintex.h"

//========================================================================

struct App
{
   TFile              *m_file;
   TTree              *m_event_tree;
   fwlite::Event      *m_event;

   edm::Handle<edm::SimTrackContainer>    m_tracksH;
   edm::Handle<edm::SimVertexContainer>  m_verticesH;
   edm::Handle<reco::GenParticleCollection> m_genParticlesH;
   //------------------------------------------------------

   App(const char* fname)
   {
      m_file = TFile::Open(fname);
      m_event_tree = dynamic_cast<TTree*>(m_file->Get("Events"));
      m_event = 0;
      try
      {
         m_event = new fwlite::Event(m_file);


         edm::InputTag tagS("g4SimHits");
         m_event->getByLabel(tagS, m_tracksH);
         m_event->getByLabel(tagS, m_verticesH);

         edm::InputTag tagG("genParticles");
         m_event->getByLabel(tagG, m_genParticlesH);

      }
      catch (const cms::Exception& iE)
      {
         printf("can't create a fwlite::Event\n");
         std::cerr << iE.what() <<std::endl;
         throw;
      }

   }

   ~App()
   {
      delete m_event;
      delete m_file;
   }

   //------------------------------------------------------

   void goto_event(Long64_t tid)
   {
      m_event->to(tid);
      m_event_tree->LoadTree(tid);
   }

   int getTrackWithGenId(size_t id)
   {
      for (size_t i = 0; i < m_tracksH.product()->size(); ++i)
      {
         if (m_tracksH.product()->at(i).trackId() == id)
            return int(i);
      }
      return -1;
   }

   void print_track_parent(const SimTrack &track)
   {
      int vertexId = track.vertIndex();
      const SimVertex &vert = m_verticesH.product()->at(vertexId);
      int gen4ParentIdx = vert.parentIndex();
      int np = getTrackWithGenId(gen4ParentIdx);
      if (np >= 0)
      {
         const SimTrack &p = m_tracksH.product()->at(np);
      int ppid =  m_verticesH.product()->at(p.vertIndex()).processType();

       //  printf("parent pdgId = %d \n", p.pdgId());
         printf( " parent track pt = %f, vertex idx = %d, vertex process = %d \n " , p.momentum().Pt(), p.vertIndex(), ppid);
         print_track_parent(p);
      }
   }
   //------------------------------------------------------
   void print_cascade()
   {
      for (size_t i = 0; i < m_tracksH.product()->size(); ++i)
      {
         int simTrackId = i;
         printf("\n===================\n tracing origin of SimTrack %d\n", simTrackId);
         const SimTrack &track = m_tracksH.product()->at(simTrackId);
         print_track_parent(track);
      }
   }
};
//========================================================================

int main(int argc, char* argv[])
{
   if (argc < 2)
   {
      std::cout << "Need input file!" << std::endl;
      return 1;
   }

 //  ROOT::Cintex::Cintex::Enable();

   App app(argv[1]);

   app.goto_event(0);

   app.print_cascade();

   return 0;
}
