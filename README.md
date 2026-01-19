# Green Pass
This project implements a client/server architecture for managing Green Passes.

## Description
The Green Pass management provides this operations:
- Creation
- Validity check
- Validation
- Invalidation

The creation of a Green Pass requires the user to insert his fiscal code, which will then be transmitted to an available vaccination center. 
The vaccination center will then create the Green Pass by sending a validity date associated to the fiscal code to ServerV. 
ServerV will store the indormations into a database.

For verifying the validity of a Green Pass, the user sends his fiscal code to a ServerG, which will request the period of
validity from ServerV.

Similar to the Green Pass verification phase, in both the validation/invalidation the client send the fiscal code to ServerG, which will request validation and invalidation from ServerV. 

The project implements a single client (for both the user and the vaccinal centre) capable of requesting the four operations necessary for Green Pass's management.

![Architecture visualization](architecture.png)

## Protocol

### Request
A basic request operation performs this operations:

![Basic request](req_basic.png)


In case of a request that needs additional data to be sent:

![Extended request](req_extended.png)

### Request codes

| Code | Name | Description
| ----------- | ----------- | ----------- |
| 0 | Verify | Verify a validity of a Green Pass |
| 1 | Invalidation | Invalidate a Green Pass |
| 2 | Validation | Validate a Green Pass |
| 3 | Create request | Requests a Green Pass creation |
| 4 | Creation | Creates a Green Pass |

### Request data structure
![Request data](req_data.png)

### Response codes

| Code | Name | Description
| ----------- | ----------- | ----------- |
| -3 | Server error | The server malfunctioned during the request|
| -2 | Invalid data | The data sent is not valid |
| -1 | Not implemented | The operation is not implemented |
| 0 | Not valid | Green Pass is not valid |
| 1 | Additional data | The request needs additional data |

