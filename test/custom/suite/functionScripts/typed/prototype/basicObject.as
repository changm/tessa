function TreeNode(left:String, right:String) {
    this.left = left;
    this.right = right;
}

function basicObject() {
    var x:Object = new TreeNode("aLeft", "aRight");
    print(x.left);
}

basicObject();
