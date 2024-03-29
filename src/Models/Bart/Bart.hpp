/*
  Copyright (C) 2013 Steven L. Scott

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef BOOM_BART_HPP_
#define BOOM_BART_HPP_

#include <set>
#include <distributions/rng.hpp>
#include <Models/Policies/ParamPolicy_1.hpp>
#include <Models/Policies/IID_DataPolicy.hpp>
#include <Models/Policies/PriorPolicy.hpp>
#include <Models/Glm/Glm.hpp>            // for RegressionData
#include <LinAlg/SubMatrix.hpp>

namespace BOOM {

  namespace Bart {
    class TreeNode;
    class VariableSummaryImpl;

    // The default BART algorithm operates by having each tree sample
    // a model for its data, conditional on all the other trees.  The
    // ResidualRegressionData class keeps track of the unexplained
    // variation in each data point so that each tree can do its
    // modeling.  There is one ResidualRegressionData associated with
    // each observed data point.  ResidualRegressionData is an
    // abstract class because each concrete class of BART model has
    // its own notion of what a "residual" means.
    class ResidualRegressionData;

    // Because each model has its own class of residuals, each needs
    // its own class of complete data sufficient statistics for
    // accumulating them.  The concrete sufficient statistics
    // implementations should inherit from this base class.  The main
    // duty of the base class is to model "adding" a complete data
    // observation.  Each concrete class inheriting from
    // SufficientStatisticsBase knows the type of data that it
    // expects.  It is an error (resulting in an exception) to feed
    // the wrong type of ResidualRegressionData to a concrete
    // descendant of SufficientStatisticsBase.
    class SufficientStatisticsBase {
     public:
      virtual ~SufficientStatisticsBase() {}
      // Create a copy of *this, with the same data.
      virtual SufficientStatisticsBase * clone() const = 0;
      virtual void clear() = 0;

      // Add relevant functions of data to the sufficient statistics
      // being modeled.
      virtual void update(const ResidualRegressionData &data) = 0;
      virtual SufficientStatisticsBase * create() const {
        SufficientStatisticsBase * ans = clone();
        ans->clear();
        return ans;
      }
    };

    // How should cutpoints be handled for continuous data?
    enum ContinuousCutpointStrategy {
      // Choose cutpoints at random from the interval between the
      // lowest and highest observed values.
      UNIFORM_CONTINUOUS,

      // Choose cutpoints at random from a discretization of the
      // interval between the lowest and highest observed values.
      UNIFORM_DISCRETE,

      // Choose cutpoints at random according to a discretization of
      // the empirical CDF.  This will put more cutpoints into regions
      // where there is more data.
      DISCRETE_QUANTILES
    };

    // A struct to hold the serialized value of a VariableSummary.
    // The is_continuous flag determines whether it is a summary for a
    // continuous or discrete variable, and the enum determines what
    // type of summary should be used if the variable is continuous.
    // The meaning of the data depends on the type of summary being
    // serialized.
    struct SerializedVariableSummary {
      bool finalized;
      int variable_number;
      bool is_continuous;
      ContinuousCutpointStrategy strategy;
      Vector data;
    };

    //======================================================================
    // A VariableSummary keeps track of the values observed in the
    // data.  When data are added to a BartModel, variable summary
    // keeps track of the values to use as potential cutpoints.  Call
    // make_current() after all the data has been observed to finalize
    // the state of the cutpoint information.
    class VariableSummary {
     public:
      // An empty VariableSummary.
      // Args:
      //   variable_number:  The index of the variable being summarized.
      VariableSummary(int variable_number);

      // Constructing from a SerializedVariableSummary produces an
      // already finalized VariableSummary.
      VariableSummary(const SerializedVariableSummary &);

      // When an observation is added to a Bart model, each
      // VariableSummary should get to observe the value of the
      // predictor corresponding to variable_number.
      void observe_value(double value);

      // Return a random cutpoint for this variable that is logically
      // possible given the cutpoints used by the ancestors of 'node'.
      // Args:
      //   rng:  A random number generator.
      //   node: The node for which a potential cutpoint is desired.
      //     If ancestors of 'node' also split on the same variable
      //     managed by *this, then the range of potential cutpoints
      //     will be restricted (or even empty, for example, you can't
      //     split twice on the same dummy variable).
      //   cutpoint: On a successful return, *cutpoint will point to
      //     the randomly generated cutpoint.
      // Returns:
      //   A flag indicating success.  If the return value is false,
      //   no cutpoint could be generated, and the value at *cutpoint
      //   should not be used.
      bool random_cutpoint(RNG &rng,
                           const TreeNode *node,
                           double *cutpoint) const;

      // This function should be called when the VariableSummary has
      // observed all the data associated with a Bart model.  When
      // finalize() is called the VariableSummary will decide what
      // type of variable is being modeled, and the concrete
      // implementation will be instantiated.
      //
      // Args:
      //   discrete_distribution_cutoff: The number of unique values a
      //     numeric variable must have before it is considered continuous.
      void finalize(int discrete_distribution_cutoff = 20,
                    ContinuousCutpointStrategy = UNIFORM_CONTINUOUS);

      // Serialize the value of this variable summary for long term
      // storage.
      SerializedVariableSummary serialize() const;

      // Rebuild *this from serialized data.
      void deserialize(const SerializedVariableSummary &serialized);
     private:
      // Checks whether finalize() has been called.  Throws an
      // exception if it has not.
      void check_finalized(const char *)const;
      int variable_number_;
      Vector observed_values_;
      boost::shared_ptr<VariableSummaryImpl> impl_;
    };

    //----------------------------------------------------------------------
    class VariableSummaryImpl {
     public:
      explicit VariableSummaryImpl(int variable_number);
      virtual ~VariableSummaryImpl() {}

      // Fill the *cutpoint pointer with the value of a randomly
      // chosen cutpoint from the summary.  Depending on the node and
      // its ancestors, the set of available cutpoints may differ from
      // the set of globally available cutpoints (because the
      // ancestors of *node might make some splits impossible).  The
      // variable and cutpoint of *node must be set before calling
      // this function.
      // Args:
      //   rng:  A random number generator.
      //   node:  The tree node for which a random cutpoint is desired.
      //   output_cutpoint: On successful output this will be set to
      //     the random cutpoint.
      // Returns:
      //   A true return value signals success.  False signals failure
      //   (e.g. no cutpoint available).
      virtual bool random_cutpoint(RNG &rng,
                                   const TreeNode *node,
                                   double *output_cutpoint)const = 0;

      int variable_index() const {return variable_index_;}

      virtual SerializedVariableSummary serialize() const = 0;
     private:
      int variable_index_;
    };

    //----------------------------------------------------------------------
    class DiscreteVariableSummary : public VariableSummaryImpl {
     public:
      explicit DiscreteVariableSummary(int variable_index,
                                       const Vector &values);
      virtual bool random_cutpoint(RNG &rng,
                                   const TreeNode *node,
                                   double *cutpoint)const;
      virtual SerializedVariableSummary serialize() const;
     protected:
      void set_cutpoint_values(const Vector &cutpoint_values);

     private:
      Vector cutpoint_values_;
    };

    //----------------------------------------------------------------------
    class ContinuousVariableSummary : public VariableSummaryImpl {
     public:
      explicit ContinuousVariableSummary(int variable_index,
                                         const Vector &values);
      virtual bool random_cutpoint(RNG &rng,
                                   const TreeNode *node,
                                   double *cutpoint)const;

      virtual SerializedVariableSummary serialize() const;
     private:
      double lo_;  // lower and upper limits for cutpoints
      double hi_;
    };

    //======================================================================
    // A TreeNode is one node in a Tree.  The node can be either a
    // leaf or an interior node.
    class TreeNode {
     public:
      friend class Tree;

      // At construction time, the node is a leaf.
      // Args:
      //   mean_value: The value to use for the mean parameter.  All
      //     nodes have mean parameters, but only leaves use them.
      //   parent: A pointer to the parent of *this.  If *this is a
      //     root then parent should point to NULL.
      TreeNode(double mean_value = 0.0, TreeNode *parent = NULL);
      ~TreeNode();

      // Returns a pointer to a new TreeNode equal to *this, with the
      // specified TreeNode as its parent.  All descendents of *this
      // are also cloned.
      TreeNode * recursive_clone(TreeNode *parent);

      // If the node is a leaf then the equality operator compares the
      // mean parameters.  If it is an interior node, it returns true
      // if (1) the variable and cutpoint values are equal and (2)
      // all children are equal.
      bool operator==(const TreeNode &rhs)const;
      bool operator!=(const TreeNode &rhs)const;

      // Returns the leaf value corresponding to the given vector,
      // which must have the correct number of dimensions.
      double predict(const Vector &x)const;
      double predict(const VectorView &x)const;
      double predict(const ConstVectorView &x)const;

      // Add children to a leaf node.  It is an error to call this
      // function on a non-leaf node.  The variable and cutpoint must
      // be set separately.  Before calling this function,
      // set_variable_and_cutpoint must be called so that the variable
      // and cutpoint to be split on are known.
      //
      // Args:
      //   left_mean_value: The parameter to use as the mean value of
      //     the left child.
      //   right_mean_value: The parameter to use as the mean value of
      //     the right child.
      void grow(double left_mean_value = 0.0, double right_mean_value = 0.0);

      // Remove all descendents of this node, and make this node a
      // leaf.  Returns the number of nodes that are pruned.
      int prune_descendants();

      bool is_leaf()const;
      bool has_no_grandchildren()const;
      int depth()const;

      // Returns true if this node is the left (right) child of its
      // parent.
      bool is_left_child()const;
      bool is_right_child()const;

      TreeNode * parent();
      const TreeNode * parent()const;
      TreeNode * left_child();
      TreeNode * right_child();

      void set_mean(double mean_value);
      double mean()const;

      // Find the range of possible cutpoints that can be chosen for
      // this node.  Starting with an initial global lower and upper
      // bound (which need not be tight), look up at ancestors that
      // split on the same variable as this node.  Ancestors for which
      // this node is a left descendent imply that all data flowing to
      // this node are <= their cutpoint, so they modify the
      // upper bound.  Likewise, ancestors for which this node is a
      // right-hand descendent modify the lower bound.
      //
      // Args:
      //   variable_index: The index of the variable for which a cut
      //     is being considered.
      //   lower_cutpoint_bound: A pointer to the lower bound on the
      //     range of cutpoints.  This should start out as the
      //     lower_bound() from the VariableSummary.  The bound will
      //     be modified if there are ancestors that split on the same
      //     variable.
      //   upper_cutpoint_bound: A pointer to the upper bound on the
      //     range of cutpoints.  This should start out as the
      //     upper_bound() from the VariableSummary corresponding to
      //     variable_index.  The bound will be modified if there are
      //     ancestors that split on the same variable.
      //
      // Returns:
      //   On output, the lower_bound and upper_bound are set equal to
      //   the values of right_hand and left_hand ancestors,
      //   respectively.  If no such ancestors exist, the bounds are
      //   not modified.
      void get_cutpoint_range(
          int variable_index,
          double *lower_cutpoint_bound,
          double *upper_cutpoint_bound)const;

      // Set the index of the variable for which this node represents
      // a split, and the value of the cutpoint to use for that
      // variable.  If x[variable_index] <= cutpoint then the
      // observation falls to the left child.  Otherwise it falls to
      // the right.  It is legal to call this function on a leaf node
      // (e.g. in an MCMC step where a split on this node is being
      // considered), but the values will only be used for prediction
      // if *this is a leaf.
      void set_variable_and_cutpoint(
          int variable_index,
          double cutpoint);

      // The index of the variable on which this node splits, and the
      // value of the cutpoint where the split occurs.
      int variable_index()const;
      double cutpoint()const;

      // Clears the vector of data managed by this node, and deletes
      // the sufficient statistics object describing the data.  If
      // recursive is true then data and sufficient statistics will be
      // removed from all descendents as well.
      void clear_data_and_delete_suf(bool recursive = true);

      // Associate this node with the given sufficient statistics object.
      // Args:
      //   suf:  The sufficient statistics object to use for *this.
      //   recursive: If true then the children of *this get populated
      //     with their own virtual copies (clones) of suf.  If false
      //     then nothing is assigned to the children.
      void populate_sufficient_statistics(SufficientStatisticsBase *suf,
                                          bool recursive = true);

      // Associate an observation with this node.
      // Args:
      //   dp:  The data point pointer to associate with this node.
      //   recursive: If true then dp is also associated with either
      //     the left or right child of *this, depending on whether
      //     dp->x() indicates that the observation should fall to the
      //     left or the right.
      void populate_data(ResidualRegressionData *dp,
                         bool recursive = true);

      // Re-compute sufficient statistics based on the current values
      // of the residuals assigned to this node.
      const SufficientStatisticsBase * compute_suf();

      // The vector of data associated with this node.
      const std::vector<ResidualRegressionData *> & data()const;

      // Remove the effect of this node on the predicted values of the
      // data associated with it.  (I.e. adjust the predictions as if
      // the mean of this node was zero).
      void remove_mean_effect();

      // Replace the effect of this node in the predicted values of
      // the data associated it.  This is the inverse operation to
      // remove_mean_effect().
      void replace_mean_effect();

      ostream & print(ostream &out)const;

      // Args:
      //   parent_id:  The id of the parent of this node.
      //   my_id: The id of this node.  This is the row in tree_matrix
      //     to be filled.
      //   tree_matrix:  A pointer to the matrix representing the
      //     tree.  It must have four columns and enough rows.
      // Returns:
      //   The next available id.
      int fill_tree_matrix_row(int parent_id,
                               int my_id,
                               Matrix *tree_matrix)const;
     private:
      // For singleton trees, it is possible for a node to be a root and
      // a leaf simultaneously.
      TreeNode *parent_;       // NULL if this is a root.
      TreeNode *left_child_;   // NULL if this is a leaf.
      TreeNode *right_child_;  // NULL if this is a leaf.
      int depth_;

      // For leaf nodes, this is the value predicted for all
      // observations landing on this leaf.  This is allocated for all
      // nodes, but only used if the node is a leaf.
      double mean_;

      // The data for a node is not owned by the node.
      std::vector<ResidualRegressionData *> data_;
      boost::shared_ptr<SufficientStatisticsBase> suf_;

      // For interior nodes predictions are made by going left if x <=
      // cutpoint_, and right if x > cutpoint_.
      int which_variable_;         // Used iff this is not a leaf.
      double cutpoint_;            // Used iff this is not a leaf.
    };

    inline ostream & operator<<(ostream &out, const TreeNode &node) {
      return node.print(out);
    }

    //======================================================================
    // A Tree is just a collection of TreeNodes, handled through the
    // root.  The class is useful because it helps clarify tree-level
    // operations vs node-level operations.  It also is a convenient
    // place to store global summaries of the tree (e.g. the set of
    // leaf nodes).
    class Tree {
     public:
      typedef std::set<TreeNode *>::iterator NodeSetIterator;
      typedef std::set<TreeNode *>::const_iterator NodeSetConstIterator;

      // Build an empty tree consisting of a single node with mean zero.
      Tree(double mean_value = 0);

      // Build a tree from a set of serialized tree nodes.  The format
      // described below is what is output by the to_matrix() member
      // function.
      // Args:
      //   tree_as_matrix: A 4-column matrix with the following
      //     structure.  Each row corresponds to a node in the
      //     tree. The row number in the matrix is the node id.  The
      //     elements of each row are:
      //     0) The node id of the parent.  If the node is a root then
      //        the lack of a parent is encoded as -1.  Two nodes that
      //        have the same parent are disambiguated by the rule
      //        that Left children are listed before right children.
      //     1) The value of the node's mean parameter.  All nodes
      //        have means, but only leaves use them.
      //     2) The index of the variable that the node splits on.  If
      //        the node is a leaf then this is -1.
      //     3) The value of the cutpoint that the node splits on.  If
      //        the node is a leaf then this is infinity().
      Tree(const Matrix &tree_as_matrix);

      // Copying or assigning a tree will copy or assign all its
      // nodes, cutpoints, etc.  No data or sufficient statistics are
      // associated with the new tree.
      Tree(const Tree &rhs);
      Tree & operator=(const Tree &rhs);
      void swap(Tree &rhs);
      ~Tree();

      // Compares the topology of the tree and the numerical values of
      // the nodes.
      bool operator==(const Tree &rhs)const;
      bool operator!=(const Tree &rhs)const;

      // Return this tree's contribution to the model prediction at x.
      double predict(const Vector &x)const;
      double predict(const VectorView &x)const;
      double predict(const ConstVectorView &x)const;

      TreeNode * root() {return root_.get();}
      const TreeNode * root() const {return root_.get();}

      // How many nodes are in this tree overall?
      int number_of_nodes()const;

      // Working with leaves.
      int number_of_leaves()const;
      // Iterators for the set of leaves.  Not guaranteed to be in any
      // particular order.
      NodeSetIterator leaf_begin();
      NodeSetIterator leaf_end();

      // Returns a uniformly random selection from among the tree's
      // leaves.  The tree cannot be empty, so there will always be at
      // least one leaf (though it might also be the root).
      TreeNode *random_leaf(RNG &rng);

      // Interior nodes whose children are both leaves are special
      // because they are candidates for a death move in the basic MH
      // algorithm.
      int number_of_parents_of_leaves()const;
      NodeSetIterator parents_of_leaves_begin();
      NodeSetIterator parents_of_leaves_end();

      // Returns a random interior node whose children are both
      // leaves.  If the tree is a single root then this function can
      // return NULL.
      TreeNode *random_parent_of_leaves(RNG &rng);

      // In order for a tree to grow at the specified leaf, the leaf
      // must have its variable, cutpoint, and cutpoint index set.
      // The leaf must be managed by this tree.  After grow() has been
      // called, the leaf will be entered into the set of nodes that
      // have no grandchildren, it will be removed from the set of
      // leaves, and its parent (if it has one) will be removed from
      // the set of nodes with no grandchildren.
      void grow(TreeNode *leaf,
                double left_mean = 0.0,
                double right_mean = 0.0);

      // Removes all descendents from node.  The node is kept (and
      // becomes a leaf).  The value of the mean parameter for *node
      // must be set separately.
      void prune_descendants(TreeNode *node);

      // Associates a polymorphic "clone" of *suf with each node in
      // the tree.
      void populate_sufficient_statistics(SufficientStatisticsBase *suf);

      // Drops the data pointer through the tree.  Each node that it
      // falls through keeps a copy of the pointer.
      void populate_data(ResidualRegressionData *data);

      // Removes the data from the nodes in the tree, and deletes the
      // sufficient statistics objects summarizing the data.
      void clear_data_and_delete_suf();

      // Remove any contribution that this tree has made towards the
      // residuals by having each leaf add its mean back into the
      // residuals.
      void remove_mean_effect();

      // Replace this tree's effect on the residuals by subtracting
      // each leaf's mean effect from the residuals for that leaf.
      void replace_mean_effect();

      ostream & print(ostream &out)const;

      // For serialization purposes, the tree can be stored as a
      // matrix.  The columns are:
      // 0) parent_id.  The parent id of root is -1.
      // 1) mean
      // 2) variable
      // 3) cutpoint
      //
      // You can identify a node as a left or right child by looking
      // at the relationship between the row number and the parent id.
      // A left child's row number is always one more than its
      // parent's id.
      Matrix to_matrix()const;

      // A conversion operator for recreating the tree from a matrix
      // created by to_matrix().
      void from_matrix(const ConstSubMatrix &tree_matrix);

     private:
      boost::shared_ptr<TreeNode> root_;
      int number_of_nodes_;
      std::set<TreeNode *> leaves_;
      std::set<TreeNode *> parents_of_leaves_;

      // A function to be called by special constructors (e.g., copy,
      // deserialization).  Iterates through each node in the tree and
      // registers it as needed with leaves_ and parents_of_leaves_.
      void register_special_nodes(TreeNode *node);
    };

    inline ostream & operator<<(ostream &out, const Tree &tree) {
      return tree.print(out);
    }

  }  // namespace Bart

  //======================================================================

  // This is the base class for concrete instances of the Bart model.
  // The base class manages the part of the model having to do with
  // trees.  It leaves the error distribution to the concrete classes.
  class BartModelBase : virtual public Model {
   public:
    // Args:
    //   number_of_trees:  The number of trees used in the model.
    //   mean: The model begins as a constant mean.  Each tree
    //     contributes an equal fraction to this mean, so each tree's
    //     contribution is 'mean' / 'number_of_trees'.
    BartModelBase(int number_of_trees, double mean = 0.0);
    BartModelBase(const BartModelBase &rhs);
    virtual ~BartModelBase() {}

    // Return the number of observations that this model has observed.
    virtual int sample_size() const = 0;

    // Predict the response associated with this set of predictors.
    // For concrete classes with non-identity link functions
    // (e.g. Poisson, logit, probit), this prediction is on the "sum
    // of trees" scale.  It should be fed through the link function to
    // turn it into a mean on the scale of the data.
    double predict(const Vector &x) const;
    double predict(const VectorView &x) const;
    double predict(const ConstVectorView &x) const;

    // The number of variables being modeled.  The dimension of 'x'.
    int number_of_variables()const;

    // The number of trees being used by the model.
    int number_of_trees()const;

    // If number_of_trees matches the current number of trees then
    // nothing is done.  If it exceeds the current number of trees
    // then extra single-node, zero mean trees are added.  If it is
    // less than the current number, then the appropriate number of
    // trees will be removed from the end of the vector of trees.
    void set_number_of_trees(int number_of_trees);

    // Rebuild an individual tree from its matrix representation.
    void rebuild_tree(int which_tree, const ConstSubMatrix &tree_matrix);
    // Rebuild the variable summaries from their serialized values.
    void set_variable_summaries(
        const std::vector<Bart::SerializedVariableSummary> &serialized);

    // After you're done adding data to the model, call
    // finalize_data() to let the variable summaries know that all
    // data has been observed.
    void finalize_data(
        int discrete_distribution_cutoff = 20,
        Bart::ContinuousCutpointStrategy strategy =
        Bart::UNIFORM_CONTINUOUS);

    // Returns the VariableSummary associated with the variable at the
    // given index.
    const Bart::VariableSummary & variable_summary(int which_variable)const;

    // Return a pointer to a specific tree.
    Bart::Tree *tree(int which_tree);
    const Bart::Tree *tree(int which_tree)const;
   protected:
    void observe_data(const ConstVectorView & predictor);
    void observe_data(const Vector & predictor);
   private:
    // If variable_summaries_ is empty, then populate it with 'dim'
    // empty elements.  If it is non_empty then throw an exception if
    // it has other than 'dim' elements.  Otherwise, do nothing.
    void check_variable_dimension(int dim);

    // Called by constructors to populate the trees_ data member with
    // the right number of single-node trees.  Note that the 'mean'
    // parameter is the total value of the mean when you sum across
    // trees.  Each tree's mean is 'mean' divided by the number of
    // trees.
    void create_trees(int number_of_trees, double mean);
    void add_trees(int number_of_additional_trees, double mean);
    void remove_trees(int number_of_trees_to_remove);

    // There is one VariableSummary for each variable in the
    // predictor set.  The variable_summaries_ are used to determine
    // the set of cutpoints available to the model.
    std::vector<Bart::VariableSummary> variable_summaries_;
    std::vector<boost::shared_ptr<Bart::Tree> > trees_;
  };

}  // namespace BOOM

#endif // BOOM_BART_HPP_
