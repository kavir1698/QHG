#include <omp.h>
#include <hdf5.h>

#include "SPopulation.cpp"
#include "Prioritizer.cpp"
#include "LayerBuf.cpp"
#include "LayerArrBuf.cpp"
#include "Action.cpp"

////////////////////////////

#include "WeightedMove.cpp"
#include "SingleEvaluator.cpp"
#include "MultiEvaluator.cpp"
#include "VerhulstVarK.cpp"
#include "RandomPair.cpp"
#include "GetOld.cpp"
#include "Genetics.cpp"
#include "GenLandDwellerPop.h"


//----------------------------------------------------------------------------
// constructor
//
GenLandDwellerPop::GenLandDwellerPop(SCellGrid *pCG, PopFinder *pPopFinder, int iLayerSize, IDGen **apIDG, uint32_t *aulState) 
    : SPopulation<GenLandDwellerAgent>(pCG, pPopFinder, iLayerSize, apIDG, aulState),
      m_bCreateGenomes(true) {

    m_adEnvWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];
    m_adMoveWeights = new double[m_pCG->m_iNumCells * (m_pCG->m_iConnectivity + 1)];

    // prepare parameters for MultiEvaluator, which will compute actual carrying capacity
    /* old
    std::map<char*, double*> mEvalInfo;
    
    char *altprefname = new char[64];
    strcpy(altprefname, "AltCapPref,AltWeight");
    mEvalInfo.insert(std::pair<char*, double*>(altprefname, (double*)m_pCG->m_pGeography->m_adAltitude));
    
    char *tempprefname = new char[64];
    strcpy(tempprefname, "TempCapPref,TempWeight");
    mEvalInfo.insert(std::pair<char*, double*>(tempprefname, (double*)m_pCG->m_pClimate->m_adAnnualMeanTemp));
    */  
     evaluatorinfos mEvalInfo;
    
    // add altitude evaluation
    SingleEvaluator<GenLandDwellerAgent> *pSEAlt = new SingleEvaluator<GenLandDwellerAgent>(this, pCG, NULL, (double*)m_pCG->m_pGeography->m_adAltitude, "AltCapPref");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("AltWeight", pSEAlt));

    // add altitude evaluation
    SingleEvaluator<GenLandDwellerAgent> *pSETemp = new SingleEvaluator<GenLandDwellerAgent>(this, pCG, NULL, (double*)m_pCG->m_pClimate->m_adAnnualMeanTemp, "TempCapPref");
    mEvalInfo.push_back(std::pair<std::string, Evaluator*>("TempWeight", pSETemp));

    m_pME = new MultiEvaluator<GenLandDwellerAgent>(this, m_pCG, m_adEnvWeights, mEvalInfo, false, &(m_pCG->m_pGeography->m_bUpdated));

    /* old
    delete[] tempprefname;
    delete[] altprefname;
    */

    GenLandDwellerAgent agent;
    m_pVerVarK = new VerhulstVarK<GenLandDwellerAgent>(this, m_pCG, m_apWELL, m_adEnvWeights, (int)qoffsetof(agent, m_iMateIndex));


    // evaluator for movement

    char *moveprefname = new char[64];
    strcpy(moveprefname, "AltMovePref");
    m_pSE = new SingleEvaluator<GenLandDwellerAgent>(this, m_pCG, m_adMoveWeights, (double*)m_pCG->m_pGeography->m_adAltitude,
                                                  moveprefname, true, &(m_pCG->m_pGeography->m_bUpdated));
    delete[] moveprefname;
    
    m_pWM = new WeightedMove<GenLandDwellerAgent>(this, m_pCG, m_apWELL, m_adMoveWeights);


    m_pPair = new RandomPair<GenLandDwellerAgent>(this, m_pCG, m_apWELL);

    m_pGO = new GetOld<GenLandDwellerAgent>(this, m_pCG);

    m_pGenetics = new Genetics<GenLandDwellerAgent>(this, m_pCG, m_pAgentController, m_apWELL);


    // adding all actions to prioritizer

    m_prio.addAction(MULTIEVAL_NAME, m_pME);
    m_prio.addAction(SINGLEEVAL_NAME, m_pSE);
    m_prio.addAction(WEIGHTEDMOVE_NAME, m_pWM);
    m_prio.addAction(VERHULSTVARK_NAME, m_pVerVarK);
    m_prio.addAction(RANDPAIR_NAME, m_pPair);
    m_prio.addAction(GETOLD_NAME, m_pGO);
    m_prio.addAction(GENETICS_NAME, m_pGenetics);

}

///----------------------------------------------------------------------------
// destructor
//
GenLandDwellerPop::~GenLandDwellerPop() {

    if (m_adEnvWeights != NULL) {
        delete[] m_adEnvWeights;
    }
    if (m_adMoveWeights != NULL) {
        delete[] m_adMoveWeights;
    }
    if (m_pWM != NULL) {
        delete m_pWM;
    }
    if (m_pME != NULL) {
        delete m_pME;
    }
    if (m_pSE != NULL) {
        delete m_pSE;
    }
    if (m_pVerVarK != NULL) {
        delete m_pVerVarK;
    }
    if (m_pGenetics != NULL) {
        delete m_pGenetics;
    }
}



///----------------------------------------------------------------------------
// setParams
//
int GenLandDwellerPop::setParams(const char *pParams) {
    int iResult = 0;

    return iResult;
}

///----------------------------------------------------------------------------
// preLoop
//
int GenLandDwellerPop::preLoop() {
    // here we should createGenomes (if this is a run started from scratch)
    int iResult = 0;
   
    if (m_bCreateGenomes) {
        // we assume the agents start at position 0
        int iNumMutations = 0;
        int iN = getNumAgentsTotal();
        
        iResult = m_pGenetics->createInitialGenomes(iN, iNumMutations);
    } else {
        if (m_pGenetics->isReady()) {
            iResult = 0;
        }
    }
    return iResult;
}


///----------------------------------------------------------------------------
// makePopSpecificOffspring
//
int GenLandDwellerPop::makePopSpecificOffspring(int iAgent, int iMother, int iFather) {
    
    int iResult = m_pGenetics->makeOffspring(iAgent, iMother, iFather);

    m_aAgents[iAgent].m_fAge = 0.0;

    return iResult;
}


///----------------------------------------------------------------------------
// writeAdditionalDataQDF
//
int GenLandDwellerPop::writeAdditionalDataQDF(hid_t hSpeciesGroup) {

    return m_pGenetics->writeAdditionalDataQDF(hSpeciesGroup);

}


///----------------------------------------------------------------------------
// addPopSpecificAgentData
//
int GenLandDwellerPop::addPopSpecificAgentData(int iAgentIndex, char **ppData) {

    return addAgentDataSingle(ppData, &m_aAgents[iAgentIndex].m_fAge);

}

///----------------------------------------------------------------------------
// addPopSpecificAgentDataTypeQDF
//
void GenLandDwellerPop::addPopSpecificAgentDataTypeQDF(hid_t *hAgentDataType) {

    GenLandDwellerAgent agent;
    H5Tinsert(*hAgentDataType, "Age", qoffsetof(agent, m_fAge), H5T_NATIVE_FLOAT);

}

///----------------------------------------------------------------------------
// readAdditionalDataQDF
//
int GenLandDwellerPop::readAdditionalDataQDF(hid_t hSpeciesGroup) {

    int iResult = m_pGenetics->readAdditionalDataQDF(hSpeciesGroup);

    if (iResult == 0) {
        // we read the genome from a QDF: no need to create it
        m_bCreateGenomes = false;
    } else {
        m_bCreateGenomes = true;
        iResult = 0;
    }

    return iResult;
}