#include <math.h>
#include <stdlib.h>
#include "image.h"
#include "matrix.h"

//Partner: Gareth Coad

// Run an activation function on each element in a matrix,
// modifies the matrix in place
// matrix m: Input to activation function
// ACTIVATION a: function to run
void activate_matrix(matrix m, ACTIVATION a)
{
    int i, j;
    for(i = 0; i < m.rows; ++i){
        double sum = 0;
        for(j = 0; j < m.cols; ++j){
            double x = m.data[i][j];
            if(a == LOGISTIC){
                // TODO
                m.data[i][j] = 1 / (1 + exp(-x));

            } else if (a == RELU){
                // TODO
                if (x > 0){
                    m.data[i][j] = m.data[i][j];
                }
                else{
                    m.data[i][j] = 0;
                }

            } else if (a == LRELU){
                // TODO
                if (x > 0){
                    m.data[i][j] = x;
                }
                else{
                    m.data[i][j] = 0.1*m.data[i][j]; //Wikipedia says 0.01 but 0.1 passes.
                }

            } else if (a == SOFTMAX){
                // TODO
                m.data[i][j] = exp(x);
            }
            sum += m.data[i][j];
        }
        if (a == SOFTMAX) {
            // TODO: have to normalize by sum if we are using SOFTMAX
            for(j = 0; j < m.cols; ++j){
                m.data[i][j] = m.data[i][j] / sum;
            }
        }
    }
}

// Calculates the gradient of an activation function and multiplies it into
// the delta for a layer
// matrix m: an activated layer output
// ACTIVATION a: activation function for a layer
// matrix d: delta before activation gradient
void gradient_matrix(matrix m, ACTIVATION a, matrix d)
{
    int i, j;
    for(i = 0; i < m.rows; ++i){
        for(j = 0; j < m.cols; ++j){
            double x = m.data[i][j];
            // TODO: multiply the correct element of d by the gradient
            if (a == LOGISTIC){
                d.data[i][j] = x*(1 - x) * d.data[i][j];
            }
            else if(a == RELU){
                if (x > 0){
                    d.data[i][j] = 1*d.data[i][j];
                }
                else{
                    d.data[i][j] = 0;
                }
            }
            else if(a == LRELU){
                if(x > 0){
                    d.data[i][j] = 1*d.data[i][j];
                }
                else{
                    d.data[i][j] = 0.1*d.data[i][j];
                }
                
            }
        }
    }
}

// Forward propagate information through a layer
// layer *l: pointer to the layer
// matrix in: input to layer
// returns: matrix that is output of the layer
matrix forward_layer(layer *l, matrix in)
{

    l->in = in;  // Save the input for backpropagation


    // TODO: fix this! multiply input by weights and apply activation function.
    //matrix out = make_matrix(in.rows, l->w.cols);

    matrix out = matrix_mult_matrix(in, l->w);
    activate_matrix(out, l->activation);


    free_matrix(l->out);// free the old output
    l->out = out;       // Save the current output for gradient calculation
    return out;
}

// Backward propagate derivatives through a layer
// layer *l: pointer to the layer
// matrix delta: partial derivative of loss w.r.t. output of layer
// returns: matrix, partial derivative of loss w.r.t. input to layer
matrix backward_layer(layer *l, matrix delta)
{
    // 1.4.1
    // delta is dL/dy
    // TODO: modify it in place to be dL/d(xw)
    gradient_matrix(l->out, l->activation, delta);


    // 1.4.2
    // TODO: then calculate dL/dw and save it in l->dw
    free_matrix(l->dw);
    //matrix dw = make_matrix(l->w.rows, l->w.cols); // replace this
    matrix dw = matrix_mult_matrix(transpose_matrix(l->in), delta);
    l->dw = dw;
    
    // 1.4.3
    // TODO: finally, calculate dL/dx and return it.
    //matrix dx = make_matrix(l->in.rows, l->in.cols); // replace this
    matrix wt = transpose_matrix(l->w);
    matrix dx = matrix_mult_matrix(delta, wt);

    return dx;
}

// Update the weights at layer l
// layer *l: pointer to the layer
// double rate: learning rate
// double momentum: amount of momentum to use
// double decay: value for weight decay
void update_layer(layer *l, double rate, double momentum, double decay)
{
    // TODO:
    // Calculate Δw_t = dL/dw_t - λw_t + mΔw_{t-1}
    matrix weight = axpy_matrix(-1*decay, l->w, l->dw);
    matrix moment = axpy_matrix(momentum, l->v, weight);

    free_matrix(weight);
    free_matrix(l->v);

    // save it to l->v
    l->v = moment;

    // Update l->w
    matrix g_loss = axpy_matrix(rate, l->v, l->w);
    free_matrix(l->w);
    l->w = g_loss;


    // Remember to free any intermediate results to avoid memory leaks

}

// Make a new layer for our model
// int input: number of inputs to the layer
// int output: number of outputs from the layer
// ACTIVATION activation: the activation function to use
layer make_layer(int input, int output, ACTIVATION activation)
{
    layer l;
    l.in  = make_matrix(1,1);
    l.out = make_matrix(1,1);
    l.w   = random_matrix(input, output, sqrt(2./input));
    l.v   = make_matrix(input, output);
    l.dw  = make_matrix(input, output);
    l.activation = activation;
    return l;
}

// Run a model on input X
// model m: model to run
// matrix X: input to model
// returns: result matrix
matrix forward_model(model m, matrix X)
{
    int i;
    for(i = 0; i < m.n; ++i){
        X = forward_layer(m.layers + i, X);
    }
    return X;
}

// Run a model backward given gradient dL
// model m: model to run
// matrix dL: partial derivative of loss w.r.t. model output dL/dy
void backward_model(model m, matrix dL)
{
    matrix d = copy_matrix(dL);
    int i;
    for(i = m.n-1; i >= 0; --i){
        matrix prev = backward_layer(m.layers + i, d);
        free_matrix(d);
        d = prev;
    }
    free_matrix(d);
}

// Update the model weights
// model m: model to update
// double rate: learning rate
// double momentum: amount of momentum to use
// double decay: value for weight decay
void update_model(model m, double rate, double momentum, double decay)
{
    int i;
    for(i = 0; i < m.n; ++i){
        update_layer(m.layers + i, rate, momentum, decay);
    }
}

// Find the index of the maximum element in an array
// double *a: array
// int n: size of a, |a|
// returns: index of maximum element
int max_index(double *a, int n)
{
    if(n <= 0) return -1;
    int i;
    int max_i = 0;
    double max = a[0];
    for (i = 1; i < n; ++i) {
        if (a[i] > max){
            max = a[i];
            max_i = i;
        }
    }
    return max_i;
}

// Calculate the accuracy of a model on some data d
// model m: model to run
// data d: data to run on
// returns: accuracy, number correct / total
double accuracy_model(model m, data d)
{
    matrix p = forward_model(m, d.X);
    int i;
    int correct = 0;
    for(i = 0; i < d.y.rows; ++i){
        if(max_index(d.y.data[i], d.y.cols) == max_index(p.data[i], p.cols)) ++correct;
    }
    return (double)correct / d.y.rows;
}

// Calculate the cross-entropy loss for a set of predictions
// matrix y: the correct values
// matrix p: the predictions
// returns: average cross-entropy loss over data points, 1/n Σ(-ylog(p))
double cross_entropy_loss(matrix y, matrix p)
{
    int i, j;
    double sum = 0;
    for(i = 0; i < y.rows; ++i){
        for(j = 0; j < y.cols; ++j){
            sum += -y.data[i][j]*log(p.data[i][j]);
        }
    }
    return sum/y.rows;
}


// Train a model on a dataset using SGD
// model m: model to train
// data d: dataset to train on
// int batch: batch size for SGD
// int iters: number of iterations of SGD to run (i.e. how many batches)
// double rate: learning rate
// double momentum: momentum
// double decay: weight decay
void train_model(model m, data d, int batch, int iters, double rate, double momentum, double decay)
{
    int e;
    for(e = 0; e < iters; ++e){
        data b = random_batch(d, batch);
        matrix p = forward_model(m, b.X);
        fprintf(stderr, "%06d: Loss: %f\n", e, cross_entropy_loss(b.y, p));
        matrix dL = axpy_matrix(-1, p, b.y); // partial derivative of loss dL/dy
        backward_model(m, dL);
        update_model(m, rate/batch, momentum, decay);
        free_matrix(dL);
        free_data(b);
    }
}


// Questions 
//
// 5.2.2.1 Why might we be interested in both training accuracy and testing accuracy? What do these two 
// numbers tell us about our current model?

/* 
    When testing our model, we collected a training and testing accuracy of 0.9041 and 0.9086 respectively. Due to how small the difference 
    in value suggest that overfitting and underfitting had little to impact on the accuracy. When testing accuracy and comparing the closest of the 
    measurements to the expected value from data it has never previously tested on before. Meanwhile testing training collects the accuracy 
    from data it has previously tested before.
*/


// 5.2.2.2 Try varying the model parameter for learning rate to different powers of 10 (i.e. 10^1, 10^0, 
// 10^-1, 10^-2, 10^-3) and training the model. What patterns do you see and how does the choice of 
// learning rate affect both the loss during training and the final model accuracy?
/*  
    We noticed that the rate of value does have an impoact on the training accuracy and we also noticed a sweet spot for getting
    good model testing and accuracy values. As we scale the rate value doen there is a negative correlation between the rate value
    and the training accurieses where as the rate value became smaller the greater the spread was in the accuracy.
*/

// 5.2.2.3 Try varying the parameter for weight decay to different powers of 10: (10^0, 10^-1, 10^-2,
// 10^-3, 10^-4, 10^-5). How does weight decay affect the final model training and test accuracy?
/*  
    We didnt notice any significant impact on the different decay values and their impact on the accuracies. When testing we found there was little to know impact on the spread in values 
    when adjusting for the different Decays. When we got values of 0.9210 and 0.9193 when the Decay was 0.00001 and when the Decay was set to 0 the values were 0.9210 and 0.9193.
    Therefore, the Decay values seems to have no significant impact on the accuracy.
*/

// 5.2.3.1 Currently the model uses a logistic activation for the first layer. Try using a the different
// activation functions we programmed. How well do they perform? What's best?
/*  
        LRELU was the most optimal
    activation  = [training, test] -> rate = 0.1 and decay = 0.00001
    Logistic    = [0.9428, 0.9397]
    RELU        = [0.9640, 0.9550]
    LRELU       = [0.9646, 0.9567]
*/

// 5.2.3.2 Using the same activation, find the best (power of 10) learning rate for your model. What is
// the training accuracy and testing accuracy?
/*  
We found that when varrying the training rate for LRELU, the value that produced the most realiable results seemed to be 0.1.
*/


// 5.2.3.3 Right now the regularization parameter `decay` is set to 0. Try adding some decay to your
// model. What happens, does it help? Why or why not may this be?
/*              LRELU
    decay = 0       = [0.9637, 0.9566]
    decay = 0.1     = [0.9571, 0.9521]
    decay = 0.01    = [0.9630, 0.9574]
    decay = 0.001   = [0.9634, 0.9553]
    decay = 0.0001  = [0.9636, 0.9554]
    decay = 0.00001 = [0.9646, 0.9567]

    Their was an inflection point as the decay settings values got smaller where the accuracy got optimally improvedand but then
    spread apart again suggesting that the values that were better choices didnt indeperaly negatively correlated with how small the decay settings got.
    There was a sweet spot to the decay settings. The lower the decay the less penalty and more overfitting. Meanwhiile, what suggested a less powerful model was a
    higher decay which meant a larger penalty for large weights. 
*/

// 5.2.3.4 Modify your model so it has 3 layers instead of two. The layers should be `inputs -> 64`,
// `64 -> 32`, and `32 -> outputs`. Also modify your model to train for 3000 iterations instead of
// 1000. Look at the training and testing error for different values of decay (powers of 10,
// 10^-4 -> 10^0). Which is best? Why?

/*  decay = 0.1      = [0.9745, 0.9683]
    decay = 0.01     = [0.9803,   0.97]
    decay = 0.001    = [0.9835, 0.9738]
    decay = 0.0001   = [0.9825, 0.9712]

    The inflection point seems to be at 0.001 because the difference in range is the smallest point.
*/

// 5.3.2.1 How well does your network perform on the CIFAR dataset?
/*  The training accuracy and testing accuracy did fareley well where on a single run of the CIFAR with a LRELU activation the values were
    [0.3457, 0.3403] which is very close in rage.
*/


