/* bankAccount.h simulates a simple bank account
 *
 * Joel Adams, Calvin College, Fall 2013.
 */


// Shared Variables
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
double bankAccountBalance = 0;

// add amount to bankAccountBalance
void deposit(double amount) {
   pthread_mutex_lock( &lock );   // before the access
   bankAccountBalance += amount;
   pthread_mutex_unlock( &lock ); // after the access
}

// subtract amount from bankAccountBalance
void withdraw(double amount) {
   pthread_mutex_lock( &lock );   // before the access
   bankAccountBalance -= amount;
   pthread_mutex_unlock( &lock ); // after the access
}

void cleanup() {
   pthread_mutex_destroy(&lock);
}

