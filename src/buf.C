/*****************************************************************************/
/*************** Implementation of the Buffer Manager Layer ******************/
/*****************************************************************************/


#include "buf.h"
#include <fstream>

ofstream ofs("out.out");


// Define buffer manager error messages here

// Define error message here
static const char* bufErrMsgs[] = { 
  // error message strings go here
  "There is no replace candidate in Buffer.",
  "There is a memory error in replacement.",
  "This page can't be found in buffer.",
  "This frame is not in replacement candidate.",
  "Can't unpin a page whose pincount equal to zero.",
  "There is no memory for new pages",
  "There is an erro when pin the first page of new pages",
  "There is an erro when free a page",
  "Can't free a page whose pincount greater than zero"
};

// Create a static "error_string_table" object and register the error messages
// with minibase system 
static error_string_table bufTable(BUFMGR,bufErrMsgs);

BufMgr::BufMgr (int numbuf, Replacer *replacer):hash_table(HTSIZE) {
  // put your code here
  bufPool = new Page[numbuf];
  bufDesc = new Descriptor[numbuf];
  hash_table.set_invaild_value(INVAILD_FRAME);
  for(int i=0;i<numbuf;++i){
    free_frames.push_back(i);
  }
}

//用于debug
void BufMgr::print(){
  ofs<<"print bufDecs"<<endl;
  for(int i=0;i<NUMBUF;++i){
    ofs<<"page num: "<<bufDesc[i].page_number<<"dirtybit: "<<bufDesc[i].dirtybit
    <<"pin_count: "<<bufDesc[i].pin_count<<endl;
  }
  ofs<<"hash table"<<endl;
  hash_table.print(ofs);
  ofs<<"hated list"<<endl;
  for(auto iter:hated){
    ofs<<iter<<" ";
  }
  ofs<<endl;
  ofs<<"loved list"<<endl;
  for(auto iter:loved){
    ofs<<iter<<" ";
  }
  ofs<<endl;
  ofs<<"free frams"<<endl;
  for(auto iter:free_frames){
    ofs<<iter<<" ";
  }
  ofs<<endl;
}

Status BufMgr::pinPage(PageId PageId_in_a_DB, Page*& page, int emptyPage) {
  // put your code here
  int frame_num = hash_table.search(PageId_in_a_DB);
  //ofs<<"frame_out:"<<frame_num<<endl;
  //不在buffer里
  if(frame_num==INVAILD_FRAME){
    //如果没有空的frame
    if(free_frames.empty()){
       if(!hated.empty()){
        frame_num = hated.back();
        hated.pop_back();
       }
       else if(!loved.empty()){
        frame_num = loved.front();
        loved.pop_front();
       }
       else{
        return MINIBASE_FIRST_ERROR(BUFMGR,NOREPLACECANDIDATE);
       }
       Status flush_status = flushPage(bufDesc[frame_num].page_number); 
       if(flush_status!=OK){
        return MINIBASE_FIRST_ERROR(BUFMGR,REPLACEMEMERR);
       }
       hash_table.erase(bufDesc[frame_num].page_number);
    } 
    //还有空的frame
    else{
      frame_num = free_frames.back();
      free_frames.pop_back();
    }
    page = bufPool+frame_num;
    bufDesc[frame_num].dirtybit = false;
    bufDesc[frame_num].page_number = PageId_in_a_DB;
    bufDesc[frame_num].pin_count = 0;
    if(!emptyPage){
      Status read_status = MINIBASE_DB->read_page(PageId_in_a_DB,page);
      if(read_status!=OK){
        return MINIBASE_CHAIN_ERROR(BUFMGR,read_status);
      }
    }
    hash_table.insert(PageId_in_a_DB,frame_num);
  }
  //在buffer里
  else{
    page = bufPool+frame_num;
    //不再是替换候选
    if(bufDesc[frame_num].pin_count==0){
      removeFromCandidate(frame_num);
    }
  }
  bufDesc[frame_num].pin_count++;
  return OK;
}//end pinPage


Status BufMgr::newPage(PageId& firstPageId, Page*& firstpage, int howmany) {
  // put your code here
  int frame_num;
  if(free_frames.empty() && loved.empty() && hated.empty()){
    return MINIBASE_FIRST_ERROR(BUFMGR,NONEWPAGEMEM);
  }
  Status allo_status = MINIBASE_DB->allocate_page(firstPageId,howmany);
  if(allo_status!=OK){
    return MINIBASE_CHAIN_ERROR(BUFMGR,allo_status);
  }
  if(pinPage(firstPageId,firstpage)!=OK){
    Status deallo_status = MINIBASE_DB->deallocate_page(firstPageId,howmany);
    if(deallo_status!=OK){
      return MINIBASE_CHAIN_ERROR(BUFMGR,deallo_status);
    } 
    return MINIBASE_FIRST_ERROR(BUFMGR,PINNEWPAGEERR);
  }
  return OK;
}

Status BufMgr::flushPage(PageId pageid) {
  // put your code here
  int frame_num = hash_table.search(pageid);
  if(frame_num==INVAILD_FRAME){
    return MINIBASE_FIRST_ERROR(BUFMGR,PAGENOTFOUND);
  }
  if(bufDesc[frame_num].dirtybit){
    Status write_status = MINIBASE_DB->write_page(pageid,bufPool+frame_num);
    if(write_status!=OK){
      return MINIBASE_FIRST_ERROR(BUFMGR,write_status);
    }
  }
  return OK;
}
    
	  
//*************************************************************
//** This is the implementation of ~BufMgr
//************************************************************
BufMgr::~BufMgr(){
  // put your code here
  flushAllPages();
  delete[] bufDesc;
  delete[] bufPool;
}


//*************************************************************
//** This is the implementation of unpinPage
//************************************************************

Status BufMgr::unpinPage(PageId page_num, int dirty=FALSE, int hate = FALSE){
  // put your code here
  int frame_num = hash_table.search(page_num);
  if(frame_num==INVAILD_FRAME){
    return MINIBASE_FIRST_ERROR(BUFMGR,PAGENOTFOUND);
  }
  if(bufDesc[frame_num].pin_count<=0){
    return MINIBASE_FIRST_ERROR(BUFMGR,PINCOUNTERR);
  }
  if(bufDesc[frame_num].pin_count==1){
    if(hate){
      hated.push_back(frame_num);
    }
    else{
      loved.push_back(frame_num);
    }
  }
  bufDesc[frame_num].pin_count--;
  bufDesc[frame_num].dirtybit = dirty;
  return OK;
}

//*************************************************************
//** This is the implementation of freePage
//************************************************************

Status BufMgr::freePage(PageId globalPageId){
  // put your code here
  int frame_num = hash_table.search(globalPageId);
  if(frame_num!=INVAILD_FRAME){
    if(bufDesc[frame_num].pin_count==0){
      if(removeFromCandidate(frame_num)!=OK){
        return MINIBASE_FIRST_ERROR(BUFMGR,FREEPAGEERR);
      }
      hash_table.erase(globalPageId);
      bufDesc[frame_num].dirtybit = false;
      bufDesc[frame_num].page_number = INVALID_PAGE;
      bufDesc[frame_num].pin_count = 0;
      free_frames.push_back(frame_num);
    }
    else{
      return MINIBASE_FIRST_ERROR(BUFMGR,FREEPINEDPAGE);
    }
  }
  Status deallo_status = MINIBASE_DB->deallocate_page(globalPageId);
  if(deallo_status!=OK){
    return MINIBASE_CHAIN_ERROR(BUFMGR,deallo_status);
  }
  return OK;
}

Status BufMgr::flushAllPages(){
  //put your code here
  for(int i=0;i<NUMBUF;++i){
    if(bufDesc[i].page_number!=INVALID_PAGE){
      if(bufDesc[i].dirtybit){
        Status write_status = MINIBASE_DB->write_page(bufDesc[i].page_number,bufPool+i);
        if(write_status!=OK){
          return MINIBASE_CHAIN_ERROR(BUFMGR,write_status);
        }
      }
    }
  }
  return OK;
}

Status BufMgr::removeFromCandidate(int frame_num){
  bool button = false;
  for(auto iter = loved.begin();iter!=loved.end();++iter){
    if(*iter==frame_num){
      button = true;
      loved.erase(iter);
      break;
    }
  }
  if(!button){
    for(auto iter = hated.begin();iter!=hated.end();++iter){
      if(*iter==frame_num){
        button = true;
        hated.erase(iter);
        break;
      }
    }
  }
  if(button){
    return OK;
  }
  return MINIBASE_FIRST_ERROR(BUFMGR,FRAMENOTINCAN);
}