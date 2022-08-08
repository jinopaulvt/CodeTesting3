#include "FILES1.h"

void* transfer_client(void* type)

{
    FILES1* varX = (FILES1*)type;

    while(1){
        pthread_mutex_lock(&varX->process_lock);
        pthread_cond_wait(&varX->sig_lock, &varX->process_lock);

        char message[1024];
        char BufferOp[1024];
        char command[100];
        char User[100] = "nobody";

     
        snprintf(message, 1024, "0.0 Welcome to Bulletin Server\n");
        send(varX->Sok, message, strlen(message), 0);
        
        int ansiuit = 0;
        while(read(varX->Sok, BufferOp, 1024) > 0){
            memset(message, 0, 1024);
           

            sscanf(BufferOp, "%s", command);

            if (!strcmp("CLIENT", command)){
                sscanf(BufferOp, "%s %s", command, User);
                sprintf(message, "1.0 Hello %s welcome back\n", User);
                printf("%s has connected\n", User);

            }else if (!strcmp("READ", command)){
                int ansi = -1;
                sscanf(BufferOp, "%s %d", command, &ansi);
                char user_message[1024] = "";
                int ansR = 2;
               
                if (pthread_mutex_trylock(varX->read) == 0){
                    
                    pthread_mutex_lock(varX->write);
                    (*varX->read_num)++;
                    sleep(varX->read_delay);
                    varX->resources->load_board();
                    ansR = varX->resources->load_message(ansi, user_message);
                  
                    (*varX->read_num)--;
                    if (*varX->read_num == 0){
                        pthread_mutex_unlock(varX->read);
                    }
                    pthread_mutex_unlock(varX->write);

                }else{
                   
                    (*varX->read_num)++;
                    sleep(varX->read_delay);
                    varX->resources->load_board();
                    ansR = varX->resources->load_message(ansi, user_message);
                
                    (*varX->read_num)--;
                    if (*varX->read_num == 0){
                        pthread_mutex_unlock(varX->read);
                    }
                }
                
                if (ansR == 2){
                    sprintf(message, "2.2 ERROR READ  Sorry, what did u say? Or maybe Systen is IO processing\n");
                }else if(ansR){
                  
                    sprintf(message, "2.1 UNKNOWN %d message not found\n", ansi);
                }else{
                   
                    sprintf(message, "2.0 %d %s\n", ansi, user_message);
                }

            }else if (!strcmp("WRITE", command)){
                char user_sub[1024] = "";
             
                sprintf(message, "3.2 ERROR WROTE  Sorry, what did u say? Or maybe Systen is IO processing\n");
                int length1 = 0;
                int validateo = 0;
                
                for (int i = 0; BufferOp[i] != '\r' && BufferOp[i] != '\n';i++ ){
                    if (validateo){
                        user_sub[length1++] = BufferOp[i];
                        user_sub[length1] = '\0';
                    }

                    if (BufferOp[i] == ' ')
                        validateo = 1;
                }
               
                pthread_mutex_lock(varX->write);

                for (int i = 0; (i < varX->syns->get_peers_num()) ;i++)
                    varX->syns->connectto(i);
                
                
                int sync_validateo = 1;
                for (int i = 0; (i < varX->syns->get_peers_num()) && sync_validateo; i++){
                    if ( !varX->syns->get_status(i) ){
                        for (int j = 0; j < i; j++){
                            if (varX->syns->get_status(j))
                                varX->syns->calloff(j);
                        }
                        sync_validateo = 0;
                    }else{
                        if (varX->syns->commit(i)){
                            for (int j = 0; j < i; j++){
                                if (varX->syns->get_status(j))
                                    varX->syns->calloff(j);
                            }
                            sync_validateo = 0;
                        }
                    }
                }
                sleep(varX->Known_time);

                if (sync_validateo){
                    varX->resources->load_board();
                    int ansR = varX->resources->write_message(varX->resources->get_messages_num()+1, User, user_sub, length1);
                    if (ansR){
                        
                        for (int i = 0;i < varX->syns->get_peers_num(); i++)
                            if (varX->syns->get_status(i)){
                                varX->syns->calloff(i);
                            }
                    }else{
                        
                        sprintf(message, "3.0 WROTE %d\n", varX->resources->get_messages_num());
                        char userMassage_buffer[1024];
                        varX->resources->load_board();
                        varX->resources->load_message(varX->resources->get_messages_num(), userMassage_buffer);
                        if (varX->syns->debug)
                            printf("sync: commit\n");
                        for (int i = 0;i < varX->syns->get_peers_num(); i++)
                            if (varX->syns->get_status(i)){
                                varX->syns->sendmessage(i, varX->resources->get_messages_num(), userMassage_buffer);
                            }
                    }
                }

                for (int i = 0; i < varX->syns->get_peers_num(); i++)
                    varX->syns->close_connection(i);

             
                pthread_mutex_unlock(varX->write);
            

            }else if (!strcmp("REPLACE", command)){
                int ansi = -1;
                char user_str[1024] = "\0";
                sscanf(BufferOp, "%s %d", command, &ansi);
                int length1 = 0;
                int validateo = 0;
            
                for (int i = 0;BufferOp[i] != '\r' && BufferOp[i] != '\n';i++ ){
                    if (validateo){
                        user_str[length1++] = BufferOp[i];
                        user_str[length1] = '\0';
                    }

                    if (BufferOp[i] == '/')
                        validateo = 1;
                }

                sprintf(message, "3.2 ERROR WRTE  Sorry, what did u say? Or maybe Systen is INPUT/OUTPUT processing\n");
                if ((user_str[0] != '\0') && (ansi != -1) ){
                   
                    pthread_mutex_lock(varX->write);

                    for (int i = 0; (i < varX->syns->get_peers_num()) ;i++)
                        varX->syns->connectto(i);

                    varX->resources->load_board();
                    if ( (ansi > varX->resources->get_messages_num()) || (ansi <= 0) ){
                        sprintf(message, "3.1 UNKNOWN %d message not found\n", ansi);
                    }else{
            
                        int sync_validateo = 1;
                        for (int i = 0; (i < varX->syns->get_peers_num()) && sync_validateo; i++){
                            if ( !varX->syns->get_status(i) ){
                                for (int j = 0; j < i; j++){
                                    if (varX->syns->get_status(j))
                                        varX->syns->calloff(j);
                                }
                                sync_validateo = 0;
                            }else{
                                if (varX->syns->commit(i)){
                                    for (int j = 0; j < i; j++){
                                        if (varX->syns->get_status(j))
                                            varX->syns->calloff(j);
                                    }
                                    sync_validateo = 0;
                                }
                            }
                        }

                        sleep(varX->Known_time);

                        if (sync_validateo){
                            int ansR = varX->resources->write_message(ansi, User, user_str, length1);
                            if (ansR){
                        
                                for (int i = 0;i < varX->syns->get_peers_num(); i++)
                                    if (varX->syns->get_status(i)){
                                        varX->syns->calloff(i);
                                    }
                            }else{
                         
                                sprintf(message, "3.0 WROTE %d\n", ansi);
                                char userMassage_buffer[1024];
                                varX->resources->load_board();
                                varX->resources->load_message(ansi, userMassage_buffer);
                                if (varX->syns->debug)
                                    printf("sync: commit\n");
                                for (int i = 0;i < varX->syns->get_peers_num(); i++)
                                    if (varX->syns->get_status(i)){
                                        varX->syns->sendmessage(i, ansi, userMassage_buffer);
                                    }
                            }
                        }
                    }

                    for (int i = 0; (i < varX->syns->get_peers_num()) ;i++)
                        varX->syns->close_connection(i);

                  
                    pthread_mutex_unlock(varX->write);
                
                }
            }else if (!strcmp("EXIT", command)){
                ansiuit = 1;
                sprintf(message, "4.0 BYE %s\n", User);
            }
            else{
                sprintf(message, "Unknown command\n");
            }

            write(varX->Sok, message, strlen(message));

            if (ansiuit)
                break;
        }
        for (int i = 0;i < varX->syns->get_peers_num(); i++)
            if (varX->syns->get_status(i))
                varX->syns->close_connection(i);

        close(varX->Sok);
        varX->Sok = -1;
        pthread_mutex_unlock(&varX->process_lock);
        printf("%s has exited\n", User);
    }
    return NULL;
}