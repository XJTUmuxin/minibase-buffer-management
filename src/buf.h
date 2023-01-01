///////////////////////////////////////////////////////////////////////////////
/////////////  The Header File for the Buffer Manager /////////////////////////
///////////////////////////////////////////////////////////////////////////////


#ifndef BUF_H
#define BUF_H

#include "db.h"
#include "page.h"
#include<malloc.h>
#include<vector>
#include<list>

#define HASHDEBUG 0


#define NUMBUF 20   
// Default number of frames, artifically small number for ease of debugging.

#define HTSIZE 7
// Hash Table size
//You should define the necessary classes and data structures for the hash table, 
// and the queues for LSR, MRU, etc.


/*******************ALL BELOW are purely local to buffer Manager********/

// You should create enums for internal errors in the buffer manager.
enum bufErrCodes  {NOREPLACECANDIDATE,
REPLACEMEMERR,
PAGENOTFOUND,
FRAMENOTINCAN,
PINCOUNTERR,
NONEWPAGEMEM,
PINNEWPAGEERR,
FREEPAGEERR,
FREEPINEDPAGE
};

class Replacer;

class Descriptor{
public:
    PageId page_number = INVALID_PAGE;
    int pin_count = 0;
    bool dirtybit = false;
};



template<typename T1,typename T2>
class HashEntry{
public:
    T1 key;
    T2 value;
    HashEntry<T1,T2>* next;
    HashEntry<T1,T2>(const T1& _key,const T2& _value):key(_key),value(_value),next(nullptr){

    }
};

template <typename T1,typename T2>
class MiniHash{
private:
    HashEntry<T1,T2>** buckets;
    int htsize;
    T2 invaild_value;
public:
    MiniHash(int htsize){
        if(HASHDEBUG)cout<<"MiniHash construct"<<endl;
        this->htsize = htsize;
        buckets = new HashEntry<T1,T2>*[htsize]{nullptr};
    }
    ~MiniHash(){
        if(HASHDEBUG)cout<<"MiniHash destruct"<<endl;
        clear();
        delete buckets;
    }
    size_t hash_f(T1 value,int htsize){
        return (abs(value)*5+10)%htsize;
    }
    void set_invaild_value(T2 invaild_value){
        this->invaild_value = invaild_value;
    }
    void print(ostream& ofs=cout){
        if(HASHDEBUG)ofs<<"print hash_table"<<endl;
        for(int i=0;i<htsize;++i){
            ofs<<"bucket"<<i<<":"<<endl;
            HashEntry<T1,T2>* temp = buckets[i];
            int count = 0;
            while(temp!=nullptr){
                if(count>20){
                    break;
                }
                ofs<<temp->key<<" "<<temp->value<<endl;
                temp = temp->next;
                count++;
            }
        }
    }
    void insert(const T1& key,const T2& value){
        if(HASHDEBUG){
            print();
            cout<<"hash_insert "<<key<<" "<<value<<endl;
        }
        size_t index = hash_f(key,htsize);
        HashEntry<T1,T2>* temp = new HashEntry<T1,T2>(key,value);
        temp->next = buckets[index];
        buckets[index] = temp;
    }
    bool erase(const T1& key){
        if(HASHDEBUG){
            print();
            cout<<"hash_erase "<<key<<endl;
        }
        size_t index = hash_f(key,htsize);
        HashEntry<T1,T2>* temp = buckets[index];
        if(temp==nullptr)return false;
        if(temp->key==key){
            buckets[index] = temp->next;
            delete temp;
            return true;
        }
        while(temp->next!=nullptr){
            if(temp->next->key==key){
                auto temp2 = temp->next;
                temp->next = temp->next->next;
                delete temp2;
                return true;
            }
            temp = temp->next;
        }
        return false;
    }
    T2 search(const T1& key){
        if(HASHDEBUG){
            print();
            cout<<"hash_search "<<key<<endl;
        }
        size_t index = hash_f(key,htsize);
        HashEntry<T1,T2>* temp = buckets[index];
        while (temp!=nullptr)
        {
            //cout<<temp->key<<endl;
            if(temp->key==key){
                return temp->value;
            }
            temp = temp->next;
        }
        return invaild_value;
    }
    void clear(){
        if(HASHDEBUG){
            print();
            cout<<"hash clear"<<endl;
        }
        for(int i=0;i<htsize;++i){
            HashEntry<T1,T2>* temp1 = buckets[i];
            HashEntry<T1,T2>* temp2;
            while(temp1!=nullptr){
                temp2 = temp1->next;
                delete temp1;
                temp1 = temp2;
            }
            buckets[i] = nullptr;
        }
    }
};

class BufMgr {

private: // fill in this area
    Descriptor* bufDesc;
    MiniHash<PageId,int> hash_table;
    vector<int> free_frames;
    list<int> loved;
    list<int> hated;

public:

    Page* bufPool; // The actual buffer pool

    BufMgr (int numbuf, Replacer *replacer = 0); 
    // Initializes a buffer manager managing "numbuf" buffers.
	// Disregard the "replacer" parameter for now. In the full 
  	// implementation of minibase, it is a pointer to an object
	// representing one of several buffer pool replacement schemes.
    BufMgr (const BufMgr&) = delete;
    BufMgr &operator=(const BufMgr&) = delete;

    ~BufMgr();           // Flush all valid dirty pages to disk

    void print();

    Status pinPage(PageId PageId_in_a_DB, Page*& page, int emptyPage=0);
        // Check if this page is in buffer pool, otherwise
        // find a frame for this page, read in and pin it.
        // also write out the old page if it's dirty before reading
        // if emptyPage==TRUE, then actually no read is done to bring
        // the page

    Status unpinPage(PageId globalPageId_in_a_DB, int dirty, int hate);
        // hate should be TRUE if the page is hated and FALSE otherwise
        // if pincount>0, decrement it and if it becomes zero,
        // put it in a group of replacement candidates.
        // if pincount=0 before this call, return error.

    Status newPage(PageId& firstPageId, Page*& firstpage, int howmany=1); 
        // call DB object to allocate a run of new pages and 
        // find a frame in the buffer pool for the first page
        // and pin it. If buffer is full, ask DB to deallocate 
        // all these pages and return error

    Status freePage(PageId globalPageId); 
        // user should call this method if it needs to delete a page
        // this routine will call DB to deallocate the page 

    Status flushPage(PageId pageid);
        // Used to flush a particular page of the buffer pool to disk
        // Should call the write_page method of the DB class

    Status flushAllPages();
	// Flush all pages of the buffer pool to disk, as per flushPage.

    /* DO NOT REMOVE THIS METHOD */    
    Status unpinPage(PageId globalPageId_in_a_DB, int dirty=FALSE)
        //for backward compatibility with the libraries
    {
      return unpinPage(globalPageId_in_a_DB, dirty, FALSE);
    }
    Status removeFromCandidate(int frame_num);
};

#endif
