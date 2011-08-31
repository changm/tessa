function TreeNode(left, right) {
    this.left = left;
    this.right = right;
}

TreeNode.prototype.printMessage = function() {
    print(this.left);
}

var x = new TreeNode("aLeft", "aRight");
x.printMessage();
