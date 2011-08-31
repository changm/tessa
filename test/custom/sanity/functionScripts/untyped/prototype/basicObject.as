function TreeNode(left, right) {
    this.left = left;
    this.right = right;
}

function basicObject() {
    var x = new TreeNode("aLeft", "aRight");
    print(x.left);
}

basicObject();
