/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mwojtcza <mwojtcza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 13:40:27 by mwojtcza          #+#    #+#             */
/*   Updated: 2024/09/09 14:04:46 by mwojtcza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/philo.h"

// Helper function to print actions
void	print_action(t_philosopher *philo, const char *action)
{
    long timestamp;

    // Lock and check death flag
    //pthread_mutex_lock(&philo->sim->death_mutex);
    if (philo->sim->death_flag == 0)  // Only print if no philosopher has died
    {
        timestamp = current_time_in_ms() - philo->sim->start_time;
        pthread_mutex_lock(&philo->sim->log_mutex);
        printf("%ld %d %s\n", timestamp, philo->id, action);
        pthread_mutex_unlock(&philo->sim->log_mutex);
    }
   // pthread_mutex_unlock(&philo->sim->death_mutex);
}

// Helper function to pick up forks
void pick_up_forks(t_philosopher *philo)
{
    if (philo->id % 2 == 0)
    {
        pthread_mutex_lock(philo->right_fork);
        print_action(philo, "has taken a fork");
        pthread_mutex_lock(philo->left_fork);
        print_action(philo, "has taken a fork");
    }
    else
    {
        pthread_mutex_lock(philo->left_fork);
        print_action(philo, "has taken a fork");
        pthread_mutex_lock(philo->right_fork);
        print_action(philo, "has taken a fork");
    }
}

// Helper function to release forks
void release_forks(t_philosopher *philo)
{
    pthread_mutex_unlock(philo->left_fork);
    pthread_mutex_unlock(philo->right_fork);
}

// Helper function to check death condition and set the death flag if needed
int check_death(t_philosopher *philo)
{
    if (current_time_in_ms() - philo->last_meal_time > philo->sim->time_to_die)
    {
        pthread_mutex_lock(&philo->sim->death_mutex);
        if (philo->sim->death_flag == 0)
        {
            print_action(philo, "died");  // Log the death
            philo->sim->death_flag = 1;   // Set the death flag
           // printf("DEBUG: Philosopher %d died.\n", philo->id);  // Debugging message
        }
        pthread_mutex_unlock(&philo->sim->death_mutex);
        return 1;  // Philosopher has died
    }
    return 0;
}

void *philosopher_routine(void *arg)
{
    t_philosopher *philo = (t_philosopher *)arg;

    // Special case: one philosopher
    if (philo->sim->number_of_philosophers == 1)
    {
        handle_single_philosopher(philo);
        pthread_exit(NULL);
    }

    while (1)
    {
        // Check if a philosopher has already died
        pthread_mutex_lock(&philo->sim->death_mutex);
        if (philo->sim->death_flag == 1)
        {
            pthread_mutex_unlock(&philo->sim->death_mutex);
            pthread_exit(NULL);  // Exit if the death flag is set
        }
        pthread_mutex_unlock(&philo->sim->death_mutex);

        // Thinking
        print_action(philo, "is thinking");

        // Check death after thinking
        if (check_death(philo))
        {
            pthread_exit(NULL);  // Exit if the philosopher has died
        }

        // Pick up forks
        pick_up_forks(philo);

        // Check death after picking up forks
        if (check_death(philo))
        {
            pthread_exit(NULL);  // Exit if the philosopher has died
        }

        // Eating with frequent death checks
        philo->last_meal_time = current_time_in_ms();  // Update last meal time
        print_action(philo, "is eating");
        long eat_start = current_time_in_ms();
        while (current_time_in_ms() - eat_start < philo->sim->time_to_eat)
        {
            if (check_death(philo))
            {
                release_forks(philo);
                pthread_exit(NULL);  // Exit if the philosopher has died
            }
            usleep(100);  // Small sleep intervals to allow frequent death checks
        }
        philo->times_eaten++;

        // Check if the philosopher has eaten enough times
        if (philo->sim->is_optional_arg_present && philo->times_eaten >= philo->sim->number_of_times_each_philosopher_must_eat)
        {
            release_forks(philo);
            
            // Increment the finished philosophers count
            pthread_mutex_lock(&philo->sim->death_mutex);
            philo->sim->finished_philosophers++;
            if (philo->sim->finished_philosophers == philo->sim->number_of_philosophers)
            {
                philo->sim->death_flag = 1;  // Set death_flag to stop the simulation if all have finished
            }
            pthread_mutex_unlock(&philo->sim->death_mutex);

            pthread_exit(NULL);  // Exit if the philosopher has eaten enough times
        }

        // Release forks
        release_forks(philo);

        // Check death after eating
        if (check_death(philo))
        {
            pthread_exit(NULL);  // Exit if the philosopher has died
        }

        // Sleeping with frequent death checks
        print_action(philo, "is sleeping");
        long sleep_start = current_time_in_ms();
        while (current_time_in_ms() - sleep_start < philo->sim->time_to_sleep)
        {
            if (check_death(philo))
            {
                pthread_exit(NULL);  // Exit if the philosopher has died
            }
            usleep(100);  // Small sleep intervals to allow frequent death checks
        }

        // Check death after sleeping
        if (check_death(philo))
        {
            pthread_exit(NULL);  // Exit if the philosopher has died
        }
    }

    return NULL;
}


void start_simulation(t_philosopher *philosophers, t_simulation *sim)
{
    pthread_t *threads;
    threads = malloc(sizeof(pthread_t) * sim->number_of_philosophers);

    if (!threads)
        print_error("Failed to allocate memory for threads");

    for (int i = 0; i < sim->number_of_philosophers; i++)
    {
        if (pthread_create(&threads[i], NULL, philosopher_routine, &philosophers[i]) != 0)
            print_error("Failed to create philosopher thread");
    }

    // Continuously check if a philosopher has died
    while (1)
    {
        pthread_mutex_lock(&sim->death_mutex);
        if (sim->death_flag == 1)
        {
            pthread_mutex_unlock(&sim->death_mutex);
            break;  // Exit loop if a philosopher has died
        }
        pthread_mutex_unlock(&sim->death_mutex);
        usleep(10);  // Sleep for a short while before checking again
    }

    // Join all threads
    for (int i = 0; i < sim->number_of_philosophers; i++)
    {
        pthread_join(threads[i], NULL);
    }

    free(threads);
}


int main(int argc, char **argv)
{
    t_simulation sim;
    t_philosopher *philosophers;

    // Parse the arguments
    parse_arguments(argc, argv, &sim);

    // Record the start time of the simulation
    sim.start_time = current_time_in_ms();

    // Allocate memory for philosophers
    philosophers = malloc(sizeof(t_philosopher) * sim.number_of_philosophers);
    if (!philosophers)
        print_error("Failed to allocate memory for philosophers");

    // Initialize forks and philosophers
    init_forks(&sim);
    init_philosophers(philosophers, &sim);

    // Start the simulation
    start_simulation(philosophers, &sim);

    // Clean up forks and philosophers
    cleanup_forks(&sim);
    free(philosophers);

    return 0;
}
