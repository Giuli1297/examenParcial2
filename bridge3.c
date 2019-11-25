#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <string.h>





#define TO_EST 1 // representing direction to est
#define TO_WEST 2 // representing direction to west
#define TO_NOWHERE 0 // representing direction to nothing 

#define MAX_CARS 20 // maximum cars which can be used for simulation
#define MAX_CARS_BRIDGE 3 // maximum cars in single sirection on the bridge
#define MAX_CARS_BRIDGE_IN_A_ROW 4 // threshold for maximum cars in a row to a single direction  

#define true 1
#define false 0
#define place(x)  ( (TO_WEST == x) ? "West" : "Est" )
//#define indent(x)  ( (TO_WEST == x) ? "\t+++++++++++" : "\t###########" )
#define direction(random) ( (random % 2) == 0) ? TO_WEST  : TO_EST ;
//#define bridge_indent "**********"
//#define bridge_status_indent "-----------"


static char StringP[1000];
/*
* Defininng structure for a Car represntaion. It consists of a pthread, direction and its id i.e number
*/
typedef struct{
  int number;
  int direction;
  pthread_t thread;
}Car;

/*
* Bridge structure direction = current movement in the bridge
* number = total cars for a direction in bridge
* total = consecutive cars in a row for a single direction on bridge
* lock = bridge lock
* waitingForWest = number of cars in queue to West
* waitingForEst = number of cars in queue to EST
* cvar_direction = condition variable for the bridge
*/
typedef struct{
  int direction ; // direction towards which current cars on the beridge are moving to 
  int number;
  int total;
  int waitingForWest;
  int waitingForEst;
  pthread_mutex_t lock ; // required lock for mutual exclusion
  pthread_cond_t  cvar_direction;
}Bridge;

/*
* constant defining Empty Structure for a car used for resetting the value
*/
static const Car EmptyStruct;

/*
* default bridge state. initializing the bridge to default state
*/

Bridge bridge = {
  .number = 0,
  .direction = TO_NOWHERE,
  .total = 0,
  .waitingForWest = 0,
  .waitingForEst = 0,
  .lock = PTHREAD_MUTEX_INITIALIZER,
  .cvar_direction = PTHREAD_COND_INITIALIZER,
};

static int currentBridgeState=13;

/*
* major hooks (critical sections for this bridge simulaation)
*/
void print(int x);
void *OneVehicle(void* car); // If it is called it means car has arrived to the bridege and may wait for its chance to get on the bridge
void ArriveBridge(Car *car);
void OnBridge(Car *car);
void ExitBridge(Car *car);
/*
* declaration ends for core simulation methods
*/

/**
* function declarations for bridge simulation program
*/
char *strremove(char *str, const char *sub);
int getRequiredInputs();
void callEnterToContinue(void);
void createCarThreadsToWest();
void createCarThreadsToEst();
void waitForCarThreadsToFinish();
int isRepeatProgram();
void doExitProcedure();
void resetValues();
int getNewDirection();
void assignRandomDirection();
void printScreen(char *arg, int dir);
void makeStringP(void);
Car carThreadsToWest[MAX_CARS] ;
Car carThreadsToEst[MAX_CARS] ;


int carsToEst = 0 ; // maintaining count of how many cars are there for Esr
int carsToWest = 0 ; // maintaining count of how many cars are there for west


int currentCarsOnBridge = 0 ;
int currentDirection = TO_NOWHERE;

/**
* start of the program. It starts with making sure inputs are proper for simulation
* if input is proper then it creates all the car threads and wait for them to finish
* after all cars are done it will ask user to repeat the process
* Note : Conditions like maximum car in bridge, threshold for continuous movement in single direction and max cars for simulation
*        can be changed by chaning these 
*         #define MAX_CARS 15
*         #define MAX_CARS_BRIDGE 3
*         #define MAX_CARS_BRIDGE_IN_A_ROW 5
**/
int main(int argc, char const *argv[]) {
  printf("Bienvenido al simulador de puente de un sentido \n");
  do{
    resetValues();
    if(getRequiredInputs()){
      callEnterToContinue();
      assignRandomDirection();
      createCarThreadsToWest();
      createCarThreadsToEst();
      makeStringP();
      waitForCarThreadsToFinish();
    }else{
      printf("inputs are mandatory \n");
    }
  }while(isRepeatProgram());
  doExitProcedure();
  return 0;
}
/*Main program ends */
void assignRandomDirection(){
  bridge.direction = getNewDirection();
}
/**
* get required inputs for the bridge simulation to work
*/ 
int getRequiredInputs(){
  int status = false;
  printf("Ingrese numero de autos que van al oeste maxvalue(%d)  \n",MAX_CARS);
  scanf("%d", &carsToWest );
  getchar();
  while(carsToWest <= 0 || carsToWest > MAX_CARS ){
    printf("Por favor ingrese datos entre %d y %d)  \n",1,MAX_CARS);
    scanf("%d", &carsToWest );
    getchar();
  }
  printf("Ingrese el numero de autos que van al este, maxvalue(%d) \n",MAX_CARS);
  scanf("%d", &carsToEst );
  getchar();
  while(carsToEst <= 0 || carsToEst > MAX_CARS ){
    printf("Por favor ingrese datos entre %d y %d)  \n",1,MAX_CARS);
    scanf("%d", &carsToEst );
    getchar();
  }
  printf("Autos al oeste %d, Autos al este %d \n",carsToWest,carsToEst);
  status = true;
  return status;
}
/**
* Create all the threads for simulating cars going to West
*/
void createCarThreadsToWest(){
  int carsCount = 0 ;
  for (carsCount = 0; carsCount < carsToWest; carsCount++) {
    carThreadsToWest[carsCount].number = carsCount;
    carThreadsToWest[carsCount].direction = TO_WEST ;
    pthread_create(&carThreadsToWest[carsCount].thread,NULL,OneVehicle,(void*)&carThreadsToWest[carsCount]);
  }
}
/**
* Create all the threads for simulating cars going to Est
*/
void createCarThreadsToEst(){
  int carsCount = 0 ;
  for (carsCount = 0; carsCount < carsToEst; carsCount++) {
    carThreadsToEst[carsCount].number = carsCount;
    carThreadsToEst[carsCount].direction = TO_EST;
    pthread_create(&carThreadsToEst[carsCount].thread,NULL,OneVehicle,(void*)&carThreadsToEst[carsCount]);
  }
}
/**
* this function will make sure that main program exits only when all the child thread are done 
*/
void waitForCarThreadsToFinish(){
  int carsCount = 0 ;
  for (carsCount = 0; carsCount < carsToWest; carsCount++) {
    pthread_join(carThreadsToWest[carsCount].thread, NULL);
  }
  for (carsCount = 0; carsCount < carsToEst; carsCount++) {
    pthread_join(carThreadsToEst[carsCount].thread, NULL);
  }

}
/**
* This method is called when the vehicle arrives and following things are performed here
* 1. Checks the direction towards which vehicle is going and parse the car structure
* 2. Call Arrive Bridge (it doesn't returns untill conditions are right )
* 3. Call OnBridge which will print status of the bridge and checks the Assert conditions
* 4. Exit the bridge prining car number and direction
* Note : after call to each method ArriveBridge, OnBridge, ExitBrdge method calls sleep to promote interleavings with other threads, 
*         using this sleep it can simulate 3 cars at a time at bidge
*/
void *OneVehicle(void* vargs) {
  Car *car ;
  car = (Car *)vargs;
  ArriveBridge(car);
  //sleep(1); 
  OnBridge(car);
  sleep(1); // calling interleavings
  ExitBridge(car);
  //sleep(1); 
  pthread_exit(NULL);
}
/**
* When a car arrives at the bridge it checks for following things to avoid race condition and accidents and starving
* 1. get the lock for the bridge
* 2. Check the direction of the car and the bridge
* 3. wait till car is aligned to the direction and limits of the bridge
* 4. if its satisfy the condition then change the values in the bridge and then release the lock.
* Note : Following conditions are checked on the bridge 
*   Condition 1   if car direction is same as current cars running on the bridge
*     Condition 2   if bridge has threshhold for containing this car
*       Condition 3   if allwoing this will not create starving for cars waiting for other direction   
* if anyone of condition mantioned above is true then car has to wait !! 
* 
* Note : using third condition prevents starvation  
*/
void ArriveBridge(Car *car){
  pthread_mutex_lock(&bridge.lock);
  if(TO_WEST == car->direction)
    bridge.waitingForWest++;
  if(TO_EST == car->direction)
    bridge.waitingForEst++;
/*wait till the following conditions are satisfied 
* if car direction is same as current cars running on the bridge
*   if bridge has threshhold for containing this car
*     if allwoing this will not create starving for cars waiting for other direction  */
  while(bridge.direction != car->direction || bridge.number >= MAX_CARS_BRIDGE || bridge.total >= MAX_CARS_BRIDGE_IN_A_ROW){
    pthread_cond_wait(&bridge.cvar_direction, &bridge.lock);
  }
  // conditions are favorable for the cars to get on the bridge
  bridge.number++; //  register this car to bridge 
  bridge.total++;  // counter to check starvation condition,
  pthread_cond_broadcast(&bridge.cvar_direction);
  pthread_mutex_unlock(&bridge.lock);
}
/**
* simulate car's On bridge conditon also check for race condition and call assert on critical paramaeters
*/
void OnBridge(Car *car){
  pthread_mutex_lock(&bridge.lock);
  // lock was acquired to check assert condtions below
  assert((car->direction == bridge.direction));
  assert(bridge.number <= MAX_CARS_BRIDGE);
  assert(bridge.total <= MAX_CARS_BRIDGE_IN_A_ROW);
  char x[1000];
  int carN=car->number;
  sprintf(x, "\ncar %d %s", carN, place(car->direction));
  printScreen(x, car->direction);
  // releaseing lock now
  pthread_mutex_unlock(&bridge.lock);
}
/**
* get the bridge lock delete cars entry from the bridge register
* this function will also need bridge lock to change the bridge counters/register
* 
*/
void ExitBridge(Car *car){
  pthread_mutex_lock(&bridge.lock);
  char x[1000];
  int carN=car->number;
  sprintf(x, "\ncar %d %s", carN, place(car->direction));
  printScreen(x, 3);
  if(TO_WEST == car->direction)
    bridge.waitingForWest--;
  if(TO_EST== car->direction)
      bridge.waitingForEst--;
  // change bridge status and decrease a number of car on the bridge  
  bridge.number--;
  // if bridge is empty then signal all the thread to get a lock of bridge.
  // if bridge is empty then ot will toggle the direction of the bridge if there are cars waiting on the other side
  if(bridge.number == 0){
    // condition for checking if any car is waiting for other direction ?? 
    // if waiting then only change the direction else continue to same direction. 
    if(TO_WEST == bridge.direction && bridge.waitingForEst > 0 ){
      bridge.direction = TO_EST ;
    }else if (TO_EST == bridge.direction && bridge.waitingForWest > 0){ //only change direction if someone is waiting in opposite direction
      bridge.direction = TO_WEST ;
    }
    bridge.total = 0 ; // reset total counter only if direction is changed
  }
  // signals other thread to check their turn to enter the bridge. 
  pthread_cond_broadcast(&bridge.cvar_direction);
  pthread_mutex_unlock(&bridge.lock);
}

int getNewDirection(){
  srand(time(NULL));
  int random = rand();
  return direction(random);

}
/**
* resets prgram values to simulate the bridge again
*/
void resetValues(){
  int carSize = 0 ;
  int maxValue = (carsToWest >= carsToEst) ? carsToWest : carsToEst ;
  for (carSize = 0; carSize < MAX_CARS; carSize++) {
    carThreadsToWest[carSize] = EmptyStruct;
    carThreadsToEst[carSize] = EmptyStruct;
  }
  carsToWest = 0 ;
  carsToEst = 0 ;
}
/*
* helper method which takes input from the user if he/she want to repeat the program. 
* accordingly it will result true/false
*/
int isRepeatProgram(){
  int result = false;
  char response ;
  printf("Ingrese 'R' para repetir, cualquier otra tecla para salir>   \n");
  scanf("%c",&response);
  getchar();
  if('r' == response || 'R' == response){
    result = true;
  }
  return result;
}
void doExitProcedure(){
  printf("Fin del Programa\n");
  callEnterToContinue();
}

/*
* showing initial tutoral for interpreting result
*/

/**
*helper method to ask user to press enter and continue
*/
void callEnterToContinue(void){
	printf("Please press enter to continue :");
		while (true){
			int c = getchar();
      if (c == '\n' || c == EOF)
				break;
			}
}
char *strremove(char *str, const char *sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}

void makeStringP(void){
  int i;
  char x[1100];
  for(i=0;i<carsToWest;i++){
     sprintf(x, "\ncar %d %s", carThreadsToWest[i].number, place(carThreadsToWest[i].direction)); 
     strcat(StringP, x);
  }
  for(i=0;i<carsToEst;i++){
     sprintf(x, "\ncar %d %s", carThreadsToEst[i].number, place(carThreadsToEst[i].direction)); 
     strcat(StringP, x);
  }
  printf("%s\n", StringP);
  sleep(5);
}



void printScreen(char *arg, int dir){
  fflush(stdout);
  if(dir==0){
    //system("clear");
    //strcat(StringP, arg);
    //printf("%s",StringP);
    //sleep(3);
    //print(currentBridgeState);
  }else if(dir==1){
    system("clear");
    printf("Fila de Autos:\n%s",StringP);
    print(currentBridgeState);
    printf("\n\n%s entrando al puente\n", arg);
    //sleep(1);
    system("clear");
    strremove(StringP, arg);
    printf("Fila de Autos:\n%s",StringP);
    if(currentBridgeState==13){
      print(1);
      currentBridgeState=1;
    }else if(currentBridgeState==1){
      print(4);
      currentBridgeState=4;
    }else if(currentBridgeState==4){
      print(6);
      currentBridgeState=6;
    }else if(currentBridgeState==2){
      print(4);
      currentBridgeState=4;
    }else if(currentBridgeState==3){
      print(14);
      currentBridgeState=14;
    }else if(currentBridgeState==5){
      print(6);
      currentBridgeState=6;
    }else if(currentBridgeState==6){
      print(6);
      currentBridgeState=6;
    }else if(currentBridgeState==14){
      print(4);
      currentBridgeState=4;
    }
    printf("\n\n%s entrando al puente\n", arg);
    sleep(2);
  }else if(dir==2){
    system("clear");
    printf("Fila de Autos:\n%s",StringP);
    print(currentBridgeState);
    printf("\n\n%s entrando al puente\n", arg);
    //sleep(1);
    system("clear");
    strremove(StringP, arg);
    printf("Fila de Autos:\n%s",StringP);
    if(currentBridgeState==13){
      print(7);
      currentBridgeState=7;
    }else if(currentBridgeState==7){
      print(10);
      currentBridgeState=10;
    }else if(currentBridgeState==10){
      print(12);
      currentBridgeState=12;
    }else if(currentBridgeState==8){
      print(10);
      currentBridgeState=10;
    }else if(currentBridgeState==9){
      print(15);
      currentBridgeState=15;
    }else if(currentBridgeState==11){
      print(12);
      currentBridgeState=12;
    }else if(currentBridgeState==12){
      print(12);
      currentBridgeState=12;
    }else if(currentBridgeState==15){
      print(10);
      currentBridgeState=10;
    }
    printf("\n\n%s entrando al puente\n", arg);
    sleep(2);
  }else if(dir==3){
    system("clear");
    printf("Fila de Autos:\n%s",StringP);
    if(currentBridgeState==6){
      print(5);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=5;
    }else if(currentBridgeState==5){
      print(3);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=3;
    }else if(currentBridgeState==3){
      print(13);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=13;
    }else if(currentBridgeState==12){
      print(11);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=11;
    }else if(currentBridgeState==11){
      print(9);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=9;
    }else if(currentBridgeState==9){
      print(13);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=13;
    }else if(currentBridgeState==1){
      print(2);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      system("clear");
      printf("%s", StringP);
      print(3);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      system("clear");
      printf("%s", StringP);
      print(13);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=13;
    }else if(currentBridgeState==7){
      print(8);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      system("clear");
      printf("%s", StringP);
      print(9);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      system("clear");
      printf("%s", StringP);
      print(13);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=13;
    }else if(currentBridgeState==2){
      print(3);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      system("clear");
      printf("%s", StringP);
      print(13);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=13;
    }else if(currentBridgeState==4){
      print(5);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      system("clear");
      printf("%s", StringP);
      print(3);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=3;
    }else if(currentBridgeState==8){
      print(9);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      system("clear");
      printf("%s", StringP);
      print(13);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=13;
    }else if(currentBridgeState==10){
      print(11);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      system("clear");
      printf("%s", StringP);
      print(9);
      printf("\n\n%s saliendo del puente\n", arg);
      sleep(2);
      currentBridgeState=9;
    }
  }
}

void print(int x){
  switch(x){
    case 1:
      printf("\n\n\n==================================================================\n================>>>>>>>>>>========================================\n================>>auto-->>========================================\n================>>>>>>>>>>========================================\n==================================================================\n");
      break;   
    case 2:
       printf("\n\n\n==================================================================\n================================>>>>>>>>>>========================\n================================>>auto-->>========================\n================================>>>>>>>>>>========================\n==================================================================\n");
       break;  
    case 3:
      printf("\n\n\n==================================================================\n=================================================>>>>>>>>>>=======\n=================================================>>auto-->>=======\n=================================================>>>>>>>>>>=======\n==================================================================\n");
      break;  
    case 4:
      printf("\n\n\n==================================================================\n================>>>>>>>>>>======>>>>>>>>>>========================\n================>>auto-->>======>>auto-->>========================\n================>>>>>>>>>>======>>>>>>>>>>========================\n==================================================================\n");
      break;  
    case 5:
      printf("\n\n\n==================================================================\n================================>>>>>>>>>>=======>>>>>>>>>>=======\n================================>>auto-->>=======>>auto-->>=======\n================================>>>>>>>>>>=======>>>>>>>>>>=======\n==================================================================\n");
      break;  
    case 6:
      printf("\n\n\n==================================================================\n================>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>=======\n================>>auto-->>======>>auto-->>=======>>auto-->>=======\n================>>>>>>>>>>======>>>>>>>>>>=======>>>>>>>>>>=======\n==================================================================\n");
      break;  
    case 7:
      printf("\n\n\n==================================================================\n=================================================<<<<<<<<<<=======\n=================================================<<auto--<<=======\n=================================================<<<<<<<<<<=======\n==================================================================\n");
      break;
    case 8:
      printf("\n\n\n==================================================================\n================================<<<<<<<<<<========================\n================================<<auto--<<========================\n================================<<<<<<<<<<========================\n==================================================================\n");
      break;
    case 9:
      printf("\n\n\n==================================================================\n================<<<<<<<<<<========================================\n================<<auto--<<========================================\n================<<<<<<<<<<========================================\n==================================================================\n");
      break;
    case 10:
      printf("\n\n\n==================================================================\n================================<<<<<<<<<<=======<<<<<<<<<<=======\n================================<<auto--<<=======<<auto--<<=======\n================================<<<<<<<<<<=======<<<<<<<<<<=======\n==================================================================\n");
      break;
    case 11:
      printf("\n\n\n==================================================================\n================<<<<<<<<<<======<<<<<<<<<<========================\n================<<auto--<<======<<auto--<<========================\n================<<<<<<<<<<======<<<<<<<<<<========================\n==================================================================\n");
      break;
    case 12:
      printf("\n\n\n==================================================================\n================<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<=======\n================<<auto--<<======<<auto--<<=======<<auto--<<=======\n================<<<<<<<<<<======<<<<<<<<<<=======<<<<<<<<<<=======\n==================================================================\n");
      break;
    case 13:
        printf("\n\n\n==================================================================\n==================================================================\n==================================================================\n==================================================================\n==================================================================\n");
        break;
    case 14:
      printf("\n\n\n==================================================================\n================>>>>>>>>>>=======================>>>>>>>>>>=======\n================>>auto-->>=======================>>auto-->>=======\n================>>>>>>>>>>=======================>>>>>>>>>>=======\n==================================================================\n");
      break;
    case 15:
      printf("\n\n\n==================================================================\n================<<<<<<<<<<=======================<<<<<<<<<<=======\n================<<auto--<<=======================<<auto--<<=======\n================<<<<<<<<<<=======================<<<<<<<<<<=======\n==================================================================\n");
      break;
    default:
      printf("Error\n");      
  }
}  
  

  
  
  
  
  
  
  

  
 
  
  
  
  
  
  
  