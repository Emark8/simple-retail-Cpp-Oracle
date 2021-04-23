-- ****************************************************************
-- Date: April 14, 2021
-- Purpose: Assignment 2 - DBS311
-- ****************************************************************

SET SERVEROUTPUT ON;

-- Procedure for finding a customer based on customer ID entered by the user

CREATE OR REPLACE PROCEDURE find_customer (
   cust_id   IN    NUMBER,
   found     OUT   NUMBER
) AS
   custo_id customers.customer_id%TYPE;
BEGIN
   SELECT
      customer_id
   INTO custo_id
   FROM
      customers
   WHERE
      customer_id = cust_id;

   IF SQL%rowcount = 1 THEN
      found := 1;
      dbms_output.put_line('Customer ID: ' || cust_id || ' exists!');
   END IF;

EXCEPTION
   WHEN no_data_found THEN
      found := 0;
      dbms_output.put_line('Customer ID: ' || cust_id || ' DOES NOT exist.');
   WHEN too_many_rows THEN
      dbms_output.put_line('Too many rows returned.');
   WHEN OTHERS THEN
      dbms_output.put_line('Unexpected error.');
END;

-- testing find_customer() --
DECLARE
   cust_id   VARCHAR(10);
   find      NUMBER;
BEGIN
   cust_id := &Customer_ID;
   find_customer(cust_id, find);
   dbms_output.put_line('Found = ' || find);
END;

--=========================================================================--

-- Procedure that looks for the given product ID in the database.

CREATE OR REPLACE PROCEDURE find_product (
   productid   IN    NUMBER,
   price       OUT   products.list_price%TYPE
) AS
   prodid products.product_id%TYPE;

BEGIN
   SELECT
      product_id,
      list_price
   INTO
      prodid,
      price
   FROM
      products
   WHERE
      product_id = productid;
   
      IF SQL%rowcount = 1 THEN
         dbms_output.put_line('Product ID: ' || prodID || ' exists!');
      END IF;

EXCEPTION
   WHEN no_data_found THEN
      price := 0;
      dbms_output.put_line('Product ID: ' || prodID || ' DOES NOT exist.');
   WHEN too_many_rows THEN
      dbms_output.put_line('Too many rows returned.');
   WHEN OTHERS THEN
      dbms_output.put_line('Unexpected error.');
END;

-- testing find_product() --
DECLARE
   prod_id   VARCHAR(10);
   cost      NUMBER;
BEGIN
   prod_id := &Product_ID;
   find_product(prod_id, cost);
   dbms_output.put_line(cost);
END;

--=========================================================================--

--This procedure inserts the following values in the orders table:
--new_order_id
--customer_id (input parameter)
--'Shipped' (The value for the order status)
--56 (The sales person ID)
--sysdate (order date which is the current date)

CREATE OR REPLACE PROCEDURE add_order (
   customer_id    IN    NUMBER,
   new_order_id   OUT   NUMBER
) AS

   order_status     orders.status%TYPE := 'Shipped';
   salesperson_id   orders.salesman_id%TYPE := 56;
   order_date       orders.order_date%TYPE := sysdate;
   
BEGIN
   SELECT
      MAX(order_id) + 1
   INTO new_order_id
   FROM
      orders;

   INSERT INTO orders VALUES (
      new_order_id,
      customer_id,
      order_status,
      salesperson_id,
      order_date
   );

   dbms_output.put_line('New Order ID # ' || new_order_id || ' inserted successfully!');
   
EXCEPTION
   WHEN no_data_found THEN
      dbms_output.put_line('No data found.');
   WHEN too_many_rows THEN
      dbms_output.put_line('Too many rows returned.');
   WHEN OTHERS THEN
      dbms_output.put_line('Unexpected error.');
END;

-- testing add_order() --

DECLARE
   customerid   VARCHAR(10);
   neworderid   NUMBER;
BEGIN
   customerid := &customer_id;
   add_order(customerid, neworderid);
   dbms_output.put_line(neworderid);
END;

--=========================================================================--

--This procedure has five IN parameters. 
--It stores the values of these parameters to the table order_items.

CREATE OR REPLACE PROCEDURE add_order_item (
   orderid     IN   order_items.order_id%TYPE,
   itemid      IN   order_items.item_id%TYPE,
   productid   IN   order_items.product_id%TYPE,
   quantity    IN   order_items.quantity%TYPE,
   price       IN   order_items.unit_price%TYPE
) AS

BEGIN
   INSERT INTO order_items VALUES (
      orderid,
      itemid,
      productid,
      quantity,
      price
   );

   dbms_output.put_line('Successfully added new transaction.');
   
EXCEPTION
  WHEN OTHERS THEN
     dbms_output.put_line('Unexpected error.');
END;
