//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: HadCalorimeterSD.cc 76474 2013-11-11 10:36:34Z gcosmo $
//
/// \file HadCalorimeterSD.cc
/// \brief Implementation of the HadCalorimeterSD class

#include "HadCalorimeterSD.hh"
#include "HadCalorimeterHit.hh"

#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HadCalorimeterSD::HadCalorimeterSD(G4String name)
: G4VSensitiveDetector(name), fHitsCollection(0), fHCID(-1)
{
    collectionName.insert("HadCalorimeterColl");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HadCalorimeterSD::~HadCalorimeterSD()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HadCalorimeterSD::Initialize(G4HCofThisEvent* hce)
{
    fHitsCollection 
      = new HadCalorimeterHitsCollection(SensitiveDetectorName,collectionName[0]);
    if (fHCID<0)
    { fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection); }
    hce->AddHitsCollection(fHCID,fHitsCollection);
    
    // fill calorimeter hits with zero energy deposition
    for (G4int iColumn=0;iColumn<10;iColumn++)
        for (G4int iRow=0;iRow<2;iRow++)
        {
            HadCalorimeterHit* hit = new HadCalorimeterHit();
            fHitsCollection->insert(hit);
        }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool HadCalorimeterSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep==0.) return true;
    
    const G4TouchableHistory* touchable = step->GetPreStepPoint()->GetTouchable();

    G4VPhysicalVolume* cellPhysical = touchable->GetVolume(2);
    G4int rowNo = cellPhysical->GetCopyNo();

    G4VPhysicalVolume* columnPhysical = touchable->GetVolume(3);
    G4int columnNo = columnPhysical->GetCopyNo();

    G4int hitID = 2*columnNo+rowNo;
    HadCalorimeterHit* hit = (*fHitsCollection)[hitID];
    
    // check if it is first touch
    if (hit->GetColumnID()<0)
    {
        hit->SetColumnID(columnNo);
        hit->SetRowID(rowNo);
        G4int depth = touchable->GetHistory()->GetDepth();
        G4AffineTransform transform 
          = touchable->GetHistory()->GetTransform(depth-2);
        transform.Invert();
        hit->SetRot(transform.NetRotation());
        hit->SetPos(transform.NetTranslation());
    }
    // add energy deposition
    hit->AddEdep(edep);
    
    return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
