/*****************************************************************************/
/*************** Implementation of the Buffer Manager Layer ******************/
/*****************************************************************************/


#include "buf.h"


// Define buffer manager error messages here
//enum bufErrCodes  {...};

// Define error message here
static const char* bufErrMsgs[] = { 
  // error message strings go here
  "There is an error in Buffer Manager Layer."
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

Status BufMgr::pinPage(PageId PageId_in_a_DB, Page*& page, int emptyPage) {
  // put your code here
  int frame_num = hash_table.search(PageId_in_a_DB);
  //不在buffer里
  if(frame_num==INVAILD_FRAME){
    //如果没有空的frame
    if(free_frames.empty()){
       
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
      MINIBASE_DB->read_page(PageId_in_a_DB,page);
    }
  }
  //在buffer里
  else{
    page = bufPool+frame_num;
    //加入替换候选
  }
  bufDesc[frame_num].pin_count++;
  return OK;
}//end pinPage


Status BufMgr::newPage(PageId& firstPageId, Page*& firstpage, int howmany) {
  // put your code here
  return OK;
}

Status BufMgr::flushPage(PageId pageid) {
  // put your code here
  return OK;
}
    
	  
//*************************************************************
//** This is the implementation of ~BufMgr
//************************************************************
BufMgr::~BufMgr(){
  // put your code here
}


//*************************************************************
//** This is the implementation of unpinPage
//************************************************************

Status BufMgr::unpinPage(PageId page_num, int dirty=FALSE, int hate = FALSE){
  // put your code here
  return OK;
}

//*************************************************************
//** This is the implementation of freePage
//************************************************************

Status BufMgr::freePage(PageId globalPageId){
  // put your code here
  return OK;
}

Status BufMgr::flushAllPages(){
  //put your code here
  return OK;
}
