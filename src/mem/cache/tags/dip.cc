/*
 * Copyright (c) 2003-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Erik Hallnor
 */

/**
 * @file
 * Definitions of DIP tag store.
 */

#include <string>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <iomanip>

#include "base/intmath.hh"
#include "debug/CacheRepl.hh"
#include "mem/cache/tags/cacheset.hh"
#include "mem/cache/tags/dip.hh"
#include "mem/cache/base.hh"
#include "sim/core.hh"


using namespace std;

// create and initialize a DIP/MRU cache structure
DIP::DIP(unsigned _numSets, unsigned _blkSize, unsigned _assoc,
         unsigned _hit_latency, double _bip_throttle)
    : numSets(_numSets), blkSize(_blkSize), assoc(_assoc),
      hitLatency(_hit_latency), bipThrottle(_bip_throttle)
{
    cout << "DIP Associativity:"<< assoc << "bipThrottle" << bipThrottle << &endl;
    // Check parameters
    if (blkSize < 4 || !isPowerOf2(blkSize)) {
        fatal("Block size must be at least 4 and a power of 2");
    }
    if (numSets <= 0 || !isPowerOf2(numSets)) {
        fatal("# of sets must be non-zero and a power of 2");
    }
    if (assoc <= 0) {
        fatal("associativity must be greater than zero");
    }
    if (hitLatency <= 0) {
        fatal("access latency must be greater than zero");
    }

    blkMask = blkSize - 1;
    setShift = floorLog2(blkSize);
    setMask = numSets - 1;
    tagShift = setShift + floorLog2(numSets);
    warmedUp = false;
    /** @todo Make warmup percentage a parameter. */
    warmupBound = numSets * assoc;

    sets = new CacheSet[numSets];
    blks = new BlkType[numSets * assoc];
    // allocate data storage in one big chunk
    numBlocks = numSets * assoc;
    dataBlks = new uint8_t[numBlocks * blkSize];
    PSEL_W=10;
    dipSetInterval=32;
    PSEL = pow(2,PSEL_W)/2;
    unsigned blkIndex = 0;       // index into blks array
    for (unsigned i = 0; i < numSets; ++i) {
        sets[i].assoc = assoc;

        sets[i].blks = new BlkType*[assoc];

        // link in the data blocks
        for (unsigned j = 0; j < assoc; ++j) {
            // locate next cache block
            BlkType *blk = &blks[blkIndex];
            blk->data = &dataBlks[blkSize*blkIndex];
            ++blkIndex;

            // invalidate new cache block
            blk->invalidate();

            //EGH Fix Me : do we need to initialize blk?

            // Setting the tag to j is just to prevent long chains in the hash
            // table; won't matter because the block is invalid
            blk->tag = j;
            blk->whenReady = 0;
            blk->isTouched = false;
            blk->size = blkSize;
            sets[i].blks[j]=blk;
            blk->set = i;
        }
    }
    cout << "DIP Cache Constructed !" << PSEL << endl;
}

DIP::~DIP()
{
    delete [] dataBlks;
    delete [] blks;
    delete [] sets;
}

DIP::BlkType*
DIP::accessBlock(Addr addr, int &lat, int master_id)
{
    Addr tag = extractTag(addr);
    unsigned set = extractSet(addr);
    BlkType *blk = sets[set].findBlk(tag);
    lat = hitLatency;
    if (blk != NULL) {
        // move this block to head of the MRU list
        sets[set].moveToHead(blk);
        DPRINTF(CacheRepl, "set %x: moving blk %x to MRU\n",
                set, regenerateBlkAddr(tag, set));
        if (blk->whenReady > curTick()
            && blk->whenReady - curTick() > hitLatency) {
            lat = blk->whenReady - curTick();
        }
        blk->refCount += 1;
    }

    return blk;
}


DIP::BlkType*
DIP::findBlock(Addr addr) const
{
    Addr tag = extractTag(addr);
    unsigned set = extractSet(addr);
    BlkType *blk = sets[set].findBlk(tag);
    return blk;
}

DIP::BlkType*
DIP::findVictim(Addr addr, PacketList &writebacks)
{
    unsigned set = extractSet(addr);
    // cout << "Finding Victim " << " : Block Addr " << addr << endl;
    // grab a replacement candidate
    BlkType *blk = sets[set].blks[assoc-1];

    if (blk->isValid()) {
        DPRINTF(CacheRepl, "set %x: selecting blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }
    int j;

    for (j = assoc-1; j >= 0; j--)  // Removing invalid blocks first from cache
    {
        // cout << "travesring set @ " << j << endl ;
        blk = sets[set].blks[j];
        if (!blk->isValid())
        {
            // cout << "Block " << j << " invalid " << endl;
            sets[set].moveToTail(blk);
            break;
        }
    }
    // cout << "Here .. " << endl;
    blk = sets[set].blks[assoc-1];
    return blk;
}

void
DIP::insertBlock(Addr addr, BlkType *blk, int master_id)
{
    if (!blk->isTouched) {
        tagsInUse++;
        blk->isTouched = true;
        if (!warmedUp && tagsInUse.value() >= warmupBound) {
            warmedUp = true;
            warmupCycle = curTick();
            cout << "-----------------------Cache is warmed up here-----------------------" << endl;
            std::cin.ignore();
        }
    }

    // If we're replacing a block that was previously valid update
    // stats for it. This can't be done in findBlock() because a
    // found block might not actually be replaced there if the
    // coherence protocol says it can't be.
    if (blk->isValid()) {
        replacements[0]++;
        totalRefs += blk->refCount;
        ++sampledRefs;
        blk->refCount = 0;

        // deal with evicted block
        assert(blk->srcMasterId < cache->system->maxMasters());
        occupancies[blk->srcMasterId]--;

        blk->invalidate();
    }

    blk->isTouched = true;
    // Set tag for new block.  Caller is responsible for setting status.
    blk->tag = extractTag(addr);

    // deal with what we are bringing in
    assert(master_id < cache->system->maxMasters());
    occupancies[master_id]++;
    blk->srcMasterId = master_id;

    unsigned set = extractSet(addr);
    //uniform_int_distribution dis(0.0, 1.0);
    double randP = ((double)rand())/(RAND_MAX);

    //  Filter LRU and LIP Lead Sets
    if (set % dipSetInterval == 0)
    {
        // Dedicated LRU Sets
        sets[set].moveToHead(blk);      //  Shifting new inserted block to MRU location
        PSEL+=1;                        // LRU PSEL Incremented
        
        if(PSEL>=pow(2,PSEL_W)) 
            PSEL = pow(2,PSEL_W)-1;  //Preventing overflow
        // << PSEL<< endl;
    }
    else if (set % dipSetInterval == dipSetInterval -1 )
    {   
        // Dedicated BIP Sets
        if (randP <= bipThrottle) {
            sets[set].moveToHead(blk);      //  Shifting new inserted block to MRU location
        } 
        else {
            sets[set].insertLRU(blk);   //  Shifting new inserted block to LRU location
        }
        PSEL-=1;                    // BIP PSEL Decremented

        if(PSEL<=0) 
            PSEL = 0;     //Preventing underflow
        //cout << PSEL<< endl;
    }
    else
    {
        //  Follower Sets Decided By MSB of 10/11 bit counter PSEL from the paper
        /************************Check MSB of PSEL******************************/
        if ((PSEL >> (PSEL_W - 1)) & 1) {    // Checking MSB of PSEL counter
		    //cout << "SET - BIP" << PSEL << endl;
            //std::cin.ignore();
            /************************If SET ------   512 and above  BIP ***************************/
            if (randP <= bipThrottle) {
               sets[set].moveToHead(blk);      //  Shifting new inserted block to MRU location
            } 
            else {
                sets[set].insertLRU(blk);   //  Shifting new inserted block to LRU location
            }


        } 
        else {
		    /************************If SET ------   0 to 511  LRU  ***************************/
            //cout << "NOT SET - LRU" << PSEL << endl;
            sets[set].moveToHead(blk);      //  Shifting new inserted block to MRU location            
        }

    } 

}

void
DIP::invalidate(BlkType *blk)
{
    assert(blk);
    assert(blk->isValid());
    tagsInUse--;
    assert(blk->srcMasterId < cache->system->maxMasters());
    occupancies[blk->srcMasterId]--;
    blk->srcMasterId = Request::invldMasterId;

    // should be evicted before valid blocks
    unsigned set = blk->set;
    sets[set].moveToTail(blk);
}

void
DIP::clearLocks()
{
    for (int i = 0; i < numBlocks; i++){
        blks[i].clearLoadLocks();
    }
}

void
DIP::cleanupRefs()
{
    for (unsigned i = 0; i < numSets*assoc; ++i) {
        if (blks[i].isValid()) {
            totalRefs += blks[i].refCount;
            ++sampledRefs;
        }
    }
}
void DIP::printSet( unsigned setIndex )
{
    cout << "Set " << setIndex << " : " << endl;
    for (size_t i = 0; i < assoc; i++)
    {
        BlkType *blk = sets[setIndex].blks[i];
        cout << "  Tag : " << hex << setw(4) << setfill('0') << blk->tag << "  ";
        cout << "  Set : " << hex << setw(4) << setfill('0') << blk->set << "  ";
        cout << "  Addr : " << hex << setw(4) << setfill('0') << regenerateBlkAddr(blk->tag, blk->set) << "  ";
        cout << "  Data : " << setw(3) << setfill('0') << blk->data[0] << "  ";
        cout << "  " << setw(2) << setfill('0') << blk->data[1] << "  ";
        cout << "  " << setw(2) << setfill('0') << blk->data[2] << "  ";
        cout << "  " << setw(2) << setfill('0') << blk->data[3] << "  ";
        cout << "  " << blk->isValid() << "  ";
        cout << "  " << setw(2) << setfill('0') << blk->status;
        cout << endl;
    }
    cout << endl;
    
}