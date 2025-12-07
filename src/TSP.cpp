#include "TSP.hxx"
#include "tsp_setup.hxx"

#include <algorithm>
//#include <new>
#include <stack>
//#include <optional>

std::ostream& operator<<(std::ostream& os, const CostMatrix& cm) {
    for (std::size_t r = 0; r < cm.size(); ++r) {
        for (std::size_t c = 0; c < cm.size(); ++c) {
            const auto& elem = cm[r][c];
            os << (is_inf(elem) ? "INF" : std::to_string(elem)) << " ";
        }
        os << "\n";
    }
    os << std::endl;

    return os;
}

/* PART 1 */

/**
 * Create path from unsorted path and last 2x2 cost matrix.
 * @return The vector of consecutive vertex.
 */
path_t StageState::get_path() {

    path_t finalPath;
    int nextCity = 0;
    bool cityInPath;

    while (finalPath.size() < matrix_.size()) {

        cityInPath = false;
        for (const auto& v : unsorted_path_) {

            if (v.row == nextCity) {

                finalPath.push_back(nextCity);
                nextCity = v.col;
                cityInPath = true;
                break;
            }
        }

        if (!cityInPath) {

            for (int i=0; i < matrix_.size(); i++) {
                if(matrix_[nextCity][i] != INF) {
                    finalPath.push_back(nextCity);
                    nextCity = i;
                    break;
                }
            }
        }
    }

    return finalPath;
}

/**
 * Get minimum values from each row and returns them.
 * @return Vector of minimum values in row.
 */
std::vector<cost_t> CostMatrix::get_min_values_in_rows() const {

    std::vector<cost_t> minValues;
    cost_t minValue;
    cost_t value;

    for (int i=0; i < matrix_.size(); i++) {
        minValue = INF;

        for (int j=0; j < matrix_.size(); j++) {
            value = matrix_[i][j];

            if (value < minValue) {
                minValue = value;
            }
        }

        minValues.push_back(minValue);
    }

    return minValues;
}

/**
 * Reduce rows so that in each row at least one zero value is present.
 * @return Sum of values reduced in rows.
 */
cost_t CostMatrix::reduce_rows() {
    std::vector<cost_t> minValues = get_min_values_in_rows();
    cost_t minValue;
    cost_t sumReduced = 0;

    for (int i=0; i < matrix_.size(); i++) {
        minValue = minValues[i];
        sumReduced += minValue;

        for (int j=0; j < matrix_.size(); j++) {
            if (matrix_[i][j] != INF) {
                matrix_[i][j] = matrix_[i][j] - minValue;
            }
        }
    }

    return sumReduced;
}

/**
 * Get minimum values from each column and returns them.
 * @return Vector of minimum values in columns.
 */
std::vector<cost_t> CostMatrix::get_min_values_in_cols() const {

    std::vector<cost_t> minValues;
    cost_t minValue;
    cost_t value;

    for (int j=0; j < matrix_.size(); j++) {
        minValue = INF;

        for (int i=0; i < matrix_.size(); i++) {
            value = matrix_[i][j];

            if (value < minValue) {
                minValue = value;
            }
        }
        
        minValues.push_back(minValue);
    }

    return minValues;
}

/**
 * Reduces rows so that in each column at least one zero value is present.
 * @return Sum of values reduced in columns.
 */
cost_t CostMatrix::reduce_cols() {
    std::vector<cost_t> minValues = get_min_values_in_cols();
    cost_t minValue;
    cost_t sumReduced = 0;

    for (int j=0; j < matrix_.size(); j++) {
        minValue = minValues[j];
        sumReduced += minValue;

        for (int i=0; i < matrix_.size(); i++) {
            if (matrix_[i][j] != INF)
            {
                matrix_[i][j] = matrix_[i][j] - minValue;
            }
        }
    }

    return sumReduced;
}

/**
 * Get the cost of not visiting the vertex_t (@see: get_new_vertex())
 * @param row
 * @param col
 * @return The sum of minimal values in row and col, excluding the intersection value.
 */
cost_t CostMatrix::get_vertex_cost(std::size_t row, std::size_t col) const {
    cost_t minRowVal = INF;
    cost_t minColVal = INF;
    
    cost_t value;

    for (int i=0; i < matrix_.size(); i++) {
        value = matrix_[row][i];

        if (value < minRowVal && i != col) {
            minRowVal = value;
        }
    }

    for (int i=0; i < matrix_.size(); i++) {
        value = matrix_[i][col];

        if (value < minColVal && i != row) {
            minColVal = value;
        }
    }

    return minRowVal + minColVal;
}

/* PART 2 */

/**
 * Choose next vertex to visit:
 * - Look for vertex_t (pair row and column) with value 0 in the current cost matrix.
 * - Get the vertex_t cost (calls get_vertex_cost()).
 * - Choose the vertex_t with maximum cost and returns it.
 * @param cm
 * @return The coordinates of the next vertex.
 */
NewVertex StageState::choose_new_vertex() {
    cost_t cost;
    cost_t maxCost = -1;
    NewVertex nextVertex;

    for (int i=0; i < matrix_.size(); i++) {
        for (int j=0; j < matrix_.size(); j++) {

            if (matrix_[i][j] == 0) {
                cost = matrix_.get_vertex_cost(i, j);

                if (cost > maxCost) {
                    maxCost = cost;
                    nextVertex = NewVertex(vertex_t(i,j), cost);
                }
            }
        }
    }

    return nextVertex;
}

/**
 * Update the cost matrix with the new vertex.
 * @param new_vertex
 */
void StageState::update_cost_matrix(vertex_t new_vertex) {
    
    int start_city = new_vertex.row;
    int end_city = new_vertex.col;

    matrix_[new_vertex.col][new_vertex.row] = INF;

    for (int i=0; i < matrix_.size(); i++) {
        matrix_[i][new_vertex.col] = INF;
        matrix_[new_vertex.row][i] = INF;
    }

    while (true) {
        const auto& left_vertex_it = std::find_if(unsorted_path_.cbegin(), unsorted_path_.cend(),
            [start_city](vertex_t vertex){return vertex.col == start_city;});
        
        if (left_vertex_it == unsorted_path_.cend()) {
            break;
        }
        else {
            start_city = left_vertex_it -> row;
        }
    }

    while (true) {
        const auto& right_vertex_it = std::find_if(unsorted_path_.cbegin(), unsorted_path_.cend(),
            [end_city](vertex_t vertex){return vertex.row == end_city;});
        
        if (right_vertex_it == unsorted_path_.cend()) {
            break;
        }

        else {
            end_city = right_vertex_it -> col;
        }
    }

    matrix_[end_city][start_city] = INF;
}

/**
 * Reduce the cost matrix.zne 
 * @return The sum of reduced values.
 */
cost_t StageState::reduce_cost_matrix() {
    cost_t reducedSum = matrix_.reduce_rows();

    for (int i=0; i < matrix_.size(); i++) {
        bool ZeroExists = false;

        for (int j=0; j < matrix_.size(); j++) {
            
            if (matrix_[i][j] == 0) {
                ZeroExists = true;
                break;
            }
        }

        if (!ZeroExists) {
            reducedSum += matrix_.reduce_cols();
            break;
        }
    }

    return reducedSum;
}

/**
 * Given the optimal path, return the optimal cost.
 * @param optimal_path
 * @param m
 * @return Cost of the path.
 */
cost_t get_optimal_cost(const path_t& optimal_path, const cost_matrix_t& m) {
    cost_t cost = 0;

    for (std::size_t idx = 1; idx < optimal_path.size(); ++idx) {
        cost += m[optimal_path[idx - 1]][optimal_path[idx]];
    }

    // Add the cost of returning from the last city to the initial one.
    cost += m[optimal_path[optimal_path.size() - 1]][optimal_path[0]];

    return cost;
}

/**
 * Create the right branch matrix with the chosen vertex forbidden and the new lower bound.
 * @param m
 * @param v
 * @param lb
 * @return New branch.
 */
StageState create_right_branch_matrix(cost_matrix_t m, vertex_t v, cost_t lb) {
    CostMatrix cm(m);
    cm[v.row][v.col] = INF;
    return StageState(cm, {}, lb);
}

/**
 * Retain only optimal ones (from all possible ones).
 * @param solutions
 * @return Vector of optimal solutions.
 */
tsp_solutions_t filter_solutions(tsp_solutions_t solutions) {
    cost_t optimal_cost = INF;
    for (const auto& s : solutions) {
        optimal_cost = (s.lower_bound < optimal_cost) ? s.lower_bound : optimal_cost;
    }

    tsp_solutions_t optimal_solutions;
    std::copy_if(solutions.begin(), solutions.end(),
                 std::back_inserter(optimal_solutions),
                 [&optimal_cost](const tsp_solution_t& s) { return s.lower_bound == optimal_cost; }
    );

    return optimal_solutions;
}

/**
 * Solve the TSP.
 * @param cm The cost matrix.
 * @return A list of optimal solutions.
 */
tsp_solutions_t solve_tsp(const cost_matrix_t& cm) {

    StageState left_branch(cm);

    // The branch & bound tree.
    std::stack<StageState> tree_lifo;

    // The number of levels determines the number of steps before obtaining
    // a 2x2 matrix.
    std::size_t n_levels = cm.size() - 2;

    tree_lifo.push(left_branch);   // Use the first cost matrix as the root.

    cost_t best_lb = INF;
    tsp_solutions_t solutions;

    while (!tree_lifo.empty()) {

        left_branch = tree_lifo.top();
        tree_lifo.pop();

        while (left_branch.get_level() != n_levels && left_branch.get_lower_bound() <= best_lb) {
            // Repeat until a 2x2 matrix is obtained or the lower bound is too high...

            if (left_branch.get_level() == 0) {
                left_branch.reset_lower_bound();
            }

            // 1. Reduce the matrix in rows and columns.
            cost_t new_cost = left_branch.reduce_cost_matrix();

            // 2. Update the lower bound and check the break condition.
            left_branch.update_lower_bound(new_cost);
            if (left_branch.get_lower_bound() > best_lb) {
                break;
            }

            // 3. Get new vertex and the cost of not choosing it.
            NewVertex new_vertex = left_branch.choose_new_vertex();

            // 4. Update the path - use append_to_path method.
            left_branch.append_to_path(new_vertex.coordinates);

            // 5. (KROK 3) Update the cost matrix of the left branch.
            left_branch.update_cost_matrix(new_vertex.coordinates);

            // 6. Update the right branch and push it to the LIFO.
            cost_t new_lower_bound = left_branch.get_lower_bound() + new_vertex.cost;
            tree_lifo.push(create_right_branch_matrix(cm, new_vertex.coordinates,
                                                      new_lower_bound));
        }

        if (left_branch.get_lower_bound() <= best_lb) {
            // If the new solution is at least as good as the previous one,
            // save its lower bound and its path.
            best_lb = left_branch.get_lower_bound();
            path_t new_path = left_branch.get_path();
            solutions.push_back({get_optimal_cost(new_path, cm), new_path});
        }
    }

    return filter_solutions(solutions); // Filter solutions to find only optimal ones.
}
