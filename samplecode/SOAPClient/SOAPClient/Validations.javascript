function validateQuantity(theForm) {
	
    if (theForm.quantity.value == ""  || theForm.quantity.value == "0") {
        alert("You must enter a product quantity greater than zero.");
        return false;
    }
        
    return true;
}

function validateForm(theForm) {

    if (theForm.firstName.value == "") {
        alert("Please enter your first name.");
        return false;
    }
    
    if (theForm.lastName.value == "") {
        alert("Please enter your last name.");
        return false;
    }

    if (theForm.pin.value == "") {
        alert("Please enter your employee PIN.");
        return false;
    }
    
    if (theForm.reason.value == "") {
        alert("Please a reason for your order.");
        return false;
    }

    if (theForm.orderCount.value == "0") {
        alert("Please select an item to order.");
        return false;
    }
    
    return true
}