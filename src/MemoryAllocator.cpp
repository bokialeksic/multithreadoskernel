#include "../h/MemoryAllocator.hpp"


MemBlock* MemoryAllocator::head = nullptr;
MemBlock* MemoryAllocator::tail = nullptr;
void MemoryAllocator::init(){
    head = (MemBlock*)HEAP_START_ADDR;
    head->next = nullptr;
    head->prev = nullptr;
    size_t total = ((size_t)HEAP_END_ADDR - (size_t)HEAP_START_ADDR) / MEM_BLOCK_SIZE;
    size_t header = (sizeof(MemBlock) + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    head->size = total - header;
    tail = head;
}

void* MemoryAllocator::mem_alloc(size_t s){
    if(s==0) return nullptr;
    //s je vec broj blokova, nema potrebe za preracunavanjem
    // implementiramo first fit algoritmom
    size_t header = (sizeof(MemBlock) + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    size_t needed = s + header;
    for(MemBlock* it = head;it;it = it->next){
        if(it->size < needed) continue;
        else if(it->size >= needed){
            /*dajem mu velicinu bloka od pocetka bloka do blokstart+s
             a pocetak slobodnog bloka pomeram na blokstart+s+1
             */
            size_t remain = it->size - needed; //gledamo koliko ostaje od tog bloka
            if(remain < header + 1){
                //da li je ostalo dovoljno slobodnog prostora,ako nije onda dajemo okrisniku ceo
                needed = it->size;
                s = needed - header; //
                remain = 0;
            }
            size_t offset_bytes = (it->size - needed) * MEM_BLOCK_SIZE;
            //KORISNIKU DAJEMO SA KRAJA SLOBODNOG PROSTORA!!!
            MemBlock* allocated = (MemBlock*)((char*)it + offset_bytes);

            allocated->size = s; //toliko korisnik stvarno koristi
            allocated->next = nullptr;
            allocated->prev = nullptr;

            //sad regulisemo slobodne blokove nakon sto smo korisniku dodelili prostor
            if(remain == 0){
                if(it->prev) {
                    it->prev->next = it->next;
                }
                else{
                    head = it->next;
                }

                if(it->next){
                    it->next->prev = it->prev;
                }
                else{
                    tail = it->prev;
                }
            }
            else{
                it->size = remain;
            }
            return (void*)((char*)allocated + sizeof(MemBlock));
        }
    }
    return nullptr;
}

int MemoryAllocator::mem_free(void *ptr) {
    if(ptr == nullptr || ptr < HEAP_START_ADDR || ptr > HEAP_END_ADDR) return -1;
    MemBlock* do_free = (MemBlock*)((char*)ptr - sizeof(MemBlock));
    //Dosli smo do zaglavlja bloka koji oslobadjamo
    size_t header = (sizeof(MemBlock) + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    do_free->size += header;
    //dodajemo mu velicinu zaglavlja jer je nismo racunali kad smo korisniku dodelili prostor
    if(head == nullptr){
        head = do_free;
        tail = do_free;
        do_free->next = nullptr;
        do_free->prev = nullptr;
        do_free->size -= header;
        return 0;
    }
    MemBlock* it = head;
    //Trazimo gde da ga umetnemo u listu slobodnih
    while(it!= nullptr && it < do_free){
        it = it->next;
    }
    if(it == head){
        do_free->next = head;
        do_free->prev = nullptr;
        head->prev = do_free;
        head = do_free;
    }
    else if(it == nullptr){//na kraj ubacujemo
        do_free->next = nullptr;
        do_free->prev = tail;
        tail->next = do_free;
        tail = do_free;
    }
    else{
        do_free->next = it;
        do_free->prev = it->prev;
        it->prev->next = do_free;
        it->prev = do_free;
    }

    //SAD POKUSAVAMO SPAJANJE SA PRETHODNIKOM I SLEDBENIKOM
    if(do_free->next){
        if((char*)do_free + (do_free->size*MEM_BLOCK_SIZE) == (char*)do_free->next){
            MemBlock* free_right = do_free->next;
            do_free->next = free_right->next;
            do_free->size += free_right->size + header;
            if(free_right->next){
                free_right->next->prev = do_free;
            }
            else{
                tail = do_free;
            }
        }
    }
    if(do_free->prev){
        if((char*)do_free->prev + ((do_free->prev->size + header)*MEM_BLOCK_SIZE) == (char*)do_free){
            MemBlock* free_left = do_free->prev;
            free_left->next = do_free->next;
            free_left->size += do_free->size;
            if(do_free->next){
                do_free->next->prev = free_left;
            }
            else{
                tail = free_left;
            }
            do_free = free_left;
        }
    }
    do_free->size-=header;
    return 0;
}

