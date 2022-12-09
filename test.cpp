#include"bits/stdc++.h"
using namespace std;
size_t hash_f(int value,int htsize){
    return (abs(value)*5+7)%htsize;
}

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
        this->htsize = htsize;
        buckets = new HashEntry<T1,T2>*[htsize]{nullptr};
    }
    void set_invaild_value(T2 invaild_value){
        this->invaild_value = invaild_value;
    }
    void insert(const T1& key,const T2& value){
        size_t index = hash_f(key,htsize);
        HashEntry<T1,T2>* temp = new HashEntry<T1,T2>(key,value);
        temp->next = buckets[index];
        buckets[index] = temp;
    }
    bool erase(const T1& key){
        size_t index = hash_f(key,htsize);
        HashEntry<T1,T2>* temp = buckets[index];
        if(temp==nullptr)return false;
        if(temp->key==key){
            buckets[index] = temp->next;
            return true;
        }
        while(temp->next!=nullptr){
            if(temp->next->key==key){
                temp->next = temp->next->next;
                return true;
            }
            temp = temp->next;
        }
        return false;
    }
    T2 search(const T1& key){
        size_t index = hash_f(key,htsize);
        HashEntry<T1,T2>* temp = buckets[index];
        while (temp!=nullptr)
        {
            if(temp->key==key)return temp->value;
            temp = temp->next;
        }
        return invaild_value;
    }
};
class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        MiniHash<int,int> hash(1000);
        hash.set_invaild_value(INT_MAX);
        for(int i=0;i<nums.size();++i){
            if(hash.search(target-nums[i])==INT_MAX){
                hash.insert(nums[i],i);
            }
            else{
                return {hash.search(target-nums[i]),i};
            }
        }
        return{-1,-1};
    }
};
int main(){
    Solution s;
    vector<int> nums = {2,7,11,15};
    vector<int> ans = s.twoSum(nums,9);
    for(auto i:ans)cout<<i<<endl;
    cout<<(-8%1000)<<endl;
}