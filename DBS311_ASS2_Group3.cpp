/* *******************************************************************
 * Date: April 14, 2021
 * Purpose: Assignment 2 - DBS311
 *********************************************************************/

#include <iostream>
#include <vector>
#include <occi.h>
#include <string>
#include <iomanip>
const int cartSize{5};

using namespace oracle::occi;

struct ShoppingCart {
	int product_id;
	double price;
	int quantity;
};


/* Template function that validates user input to make sure that the user enters the correct variable type
 * e.g. If the user enters a character instead of an integer, an error message will be displayed, and the user
 * will be prompted to enter the input again.
 */
/* @params: input variable and the message to be prompted to the user for input */
template <typename T>
void getInput(T& input, const std::string msg);

/* Function that displays the main menu */
int mainMenu();

/* Customer Login Function validates if the customer ID entered exists */
/* @params: OCCI pointer and an integer value for the customer ID */
int customerLogin(Connection* conn, int customerId);

/* This function calls the findProduct() and acts as the user's shopping cart */
/* @params: OCCI pointer and an array of type ShoppingCart */
int addToCart(Connection* conn, struct ShoppingCart cart[]);

/* This function calls the find_product procedure in the database */
/* @params: OCCI pointer and an integer value that holds the product id to be verified */
double findProduct(Connection* conn, int product_id);

/* This function receives an array of type ShoppingCart and the number of ordered items (products) */
/* @params: array of type ShoppingCart and an integer that holds the number of products in the cart */
void displayProducts(struct ShoppingCart cart[], int productCount);

/* This function updates two tables the database with the recent product purchased */
/* @params: OCCI pointer, an array of type ShoppingCart, integer variable for the customer ID and an integer for the number of products in the cart */
int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount);

//main program
int main(void)
{
	std::cout << std::fixed << std::setprecision(2);
	/* OCCI Variables */
	Environment* env = nullptr;
	Connection* conn = nullptr;

	/* Used Variables */
	std::string str;
	std::string user = "dbs311_211sa01";
	std::string pass = "30041154";
	std::string constr = "myoracle12c.senecacollege.ca:1521/oracle12c";

	int choice{0};				//user's choice in mainMenu() function
	unsigned int customerID{0};		//holds the user input's value for customer ID
	unsigned int validCustomer{0};		//holds the returned value of customerLogin() function
	int productCount{0};			//holds the returned value of addToCart() function
	ShoppingCart cartArr[cartSize];		//array with type struct ShoppingCart
	
	while (1)
	{
		//*** goto statement will transfer control in this location ***
		Home: choice = mainMenu();//invoke the mainMenu() function
		
		switch (choice)
		{
			case 1:
			{
				//exception handler for possible failed connection
				try
				{
					//create environement and Open the connection
					env = Environment::createEnvironment(Environment::DEFAULT);
					conn = env->createConnection(user, pass, constr);
					
					//send user input for validation along with the message to be prompted
					getInput(customerID, "Enter the customer ID: ");
				
					//call the customerLogin function and assign the return value to variable validCustomer
					validCustomer = customerLogin(conn, customerID);
					
					//if customer ID is invalid....
					if(!validCustomer)
					{
						//display message
						std::cout << "The customer does not exist." << std::endl;
						//terminate the database connection
						env->terminateConnection(conn);
						//terminate the OCCI environment
						Environment::terminateEnvironment(env);
						//go back to the main menu as specified in the assignment requirements
						goto Home;
					}
					
					//after a valid customer ID is entered, call the addToCart function and assign the return value to variable productCount
					productCount = addToCart(conn, cartArr);
					displayProducts(cartArr, productCount);
					checkout(conn, cartArr, customerID, productCount);

					//terminate the database connection
					env->terminateConnection(conn);
					
					//terminate the OCCI environment
					Environment::terminateEnvironment(env);
				}//end try
				
				catch (SQLException& sqlExcp)
				{
					std::cout << "Error ";
					//this output provides the error code and returns the error message string for this SQL exception.
					std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
					
				}//end catch
				break;
			}//end case 1
				
			case 0: //terminate program if user enters 0 in the main menu
				std::cout << "\nGoodbye!...\n" << std::endl;
				exit(1);
				
			default:
				std::cout << "You entered a wrong value." << std::endl;
		}//switch
	}//while loop
	return 0;
}//END main program


/* FUNCTION DEFINITIONS */

int mainMenu()
{
	int choice{0};
	
	std::cout << "**********************  Main Menu  **********************" << std::endl;
	std::cout << "1) Login" << std::endl;
	std::cout << "0) Exit" << std::endl;
	getInput(choice, "Enter an option (0-1) : ");
		
	return choice;
}//end mainMenu()


int customerLogin(Connection* conn, int customerId)
{
	int result{0};
	try
	{
		Statement* stmt = conn->createStatement();		//create the statement object stmt
		stmt->setSQL("BEGIN find_customer(:1, :2); END;");	//call the stored procedure find_customer
		stmt->setInt(1, customerId);				//pass first argument to procedure
		stmt->setInt(2, result);				//pass second argument to procedure
		stmt->executeUpdate();					//execute statement call
		result = stmt->getInt(2);				//assign the value returned by the procedure to a variable
		conn->terminateStatement(stmt);			   	//deallocate the statement object stmt

		if(result) return result;				//if find_customer returns 1, return result back to the main program
	}
	catch (SQLException& sqlExcp)
	{
		std::cout << "Error ";
		//this output provides the error code and returns the error message string for this SQL exception.
		std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}
	
	return result;			//return 0 if customer ID is invalid
}//end customerLogin()

int addToCart(Connection* conn, struct ShoppingCart cart[])
{
	int counter{0};			//counts the number of items entered
	int prodID{0};			//this variable will hold the user input's value for product ID
	double result{0.0};		//this variable will hold the result of the find_product procedure
	int quantity{0};		//quantity of products to purchase
	int option{-1};			//holds the user input's value for adding more products to the cart
	
	std::cout << "------------------ Add Products to Cart -----------------" << std::endl;
	//iterate to each shopping cart up to a maximum of 5 items as dictated by the macro cartSize
	for(int i = 0; i < cartSize; ++i)
	{
		getInput(prodID, "Enter the product ID: ");	//send user input for validation
		result = findProduct(conn, prodID);		//call and assign the return value of findProduct function to result variable
		
		if(result == 0)
		{
			std::cout << "The product does not exist. Try again..." << std::endl;
			i--;
		}
		//if product exists...
		else
		{
			counter ++;
			std::cout << "Product Price: $" << result << std::endl;
			getInput(quantity, "Enter the product Quantity: ");
			cart[i].product_id = prodID;			//assign the value of prodID to the first element of the struct ShoppingCart array
			cart[i].price = result;				//assign the price of the current product selected
			cart[i].quantity = quantity;			//assign the quantity of products that the user wants to purchase
			
			
			while(option != 1 || option != 0)
			{
				getInput(option, "Enter 1 to add more products or 0 to checkout: ");
				if(option == 0) return counter;
				else if(option == 1) break;
				std::cout << "Invalid Input. Please Try Again." << std::endl;
			}
		}
	}//end for loop
	return counter;
}//end addToCart()

double findProduct(Connection* conn, int product_id)
{
	double result{0.0};
	
	try
	{
		//create the statement object stmt
		Statement* stmt = conn->createStatement();		//create the statement object stmt
		
		stmt->setSQL("BEGIN find_product(:1, :2); END;");	//call the procedure find_product
		stmt->setInt(1, product_id);				//pass first argument to procedure
		stmt->setDouble(2, result);				//pass second argument to procedure
		stmt->executeUpdate();					//execute statement call
		result = stmt->getDouble(2);				//assign the value returned by the procedure
		conn->terminateStatement(stmt);				//deallocate the statement object stmt
	}
	catch (SQLException& sqlExcp)
	{
		std::cout << "Error ";
		//this output provides the error code and returns the error message string for this SQL exception.
		std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}
	return result;
}//end findProduct()

void displayProducts(struct ShoppingCart cart[], int productCount)
{
	double totalPrice{0.0};
	
	std::cout << "------------------   Ordered Products   -----------------" << std::endl;
	for(int i = 0; i < productCount; ++i)
	{
		std::cout << "---Item " << i+1 << std::endl;
		std::cout << "Product ID: " << cart[i].product_id << std::endl;
		std::cout << "Price: $" << cart[i].price << std::endl;
		std::cout << "Quantity: " << cart[i].quantity << std::endl;
		totalPrice += (cart[i].price * cart[i].quantity);
	}
	std::cout << "---------------------------------------------------------" << std::endl;
	std::cout << "Total: $" << totalPrice << std::endl;
}//end displayProducts()

int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount)
{
	bool ans = true;
	char choice;
	int newOrderID{0};
	
	while(ans)
	{
		getInput(choice, "Would you like to checkout? (Y/y or N/n): ");
		
		switch(choice)
		{
			case 'Y':
			case 'y':
			{
				try
				{
					//create statement object
					Statement* stmt = conn->createStatement();
					//call procedure add_order
					stmt->setSQL("BEGIN add_order(:1, :2); END;");
					stmt->setInt(1, customerId);	//pass first argument to procedure
					stmt->setInt(2, newOrderID);	//pass second argument to procedure
					stmt->executeUpdate();		//execute statement call
					newOrderID = stmt->getInt(2);	//assign the value returned by the procedure
					
					//loop through the shopping cart array
					for(int i = 0; i < productCount; ++i)
					{
						//call procedure add_order add_order_item
						stmt->setSQL("BEGIN add_order_item(:1, :2, :3, :4, :5); END;");
						stmt->setInt(1, newOrderID);			//pass first argument to procedure
						stmt->setInt(2, i+1);				//pass second argument for item_id to procedure
						stmt->setInt(3, cart[i].product_id);		//pass third argument to procedure
						stmt->setInt(4, cart[i].quantity);		//pass fourth argument to procedure
						stmt->setInt(5, cart[i].price);			//pass fifth argument to procedure
						stmt->executeUpdate();				//execute statement call
					}
					conn->terminateStatement(stmt);				//deallocate statement object
					std::cout << "The order is successfully completed." << std::endl;
					//this will break out of the loop
					ans = false;
				}
				catch (SQLException& sqlExcp)
				{
					std::cout << "Error ";
					//this output provides the error code and returns the error message string for this SQL exception.
					std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
				}
				break;
			}
				
			case 'N':
			case 'n':
			{
				std::cout << "The order is cancelled." << std::endl;
				//this will break out of the loop
				ans = false;
				break;
			}
				
			default:
				std::cout << "Wrong input. Try again..." << std::endl;
		}
	}
	return 0;
}//end checkout()

template <typename T>
void getInput(T& input, const std::string msg)
{
	std::cout << msg;
	std::cin >> input;
	
	while (std::cin.fail())
	{
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
		std::cout << "---------------------------------------------------------" << std::endl;
		std::cout << "Invalid Input. Please Try Again." << std::endl;
		std::cout << msg;
		std::cin >> input;
	}
	std::cin.clear();
	std::cin.ignore(INT_MAX, '\n');
}//end getInput()
