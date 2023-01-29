## TODO

- [ ] Extendable entry and sub routes
     - [ ] Use class operator() as a place to add more sub-routes
   - [ ] Having access to the context class in the callables
   - [ ] Having access to the request and the response
   - [ ] Termination of continuation of checking the sub-routes by parents
   - [ ] Termination of continuation of checking the entry-routes by any
         previous routes, or sub-routes.
   - [ ] Context modification
     - [ ] Sub-Route local context modification by any previous sub-routes
     - [ ] Inter-Entry-Route context modification by any previous
           (sub/entry) routes
   - [ ] Entry-Route prioritization
     - [ ] Auto prioritization
     - [ ] Manual prioritization
     - [ ] Hinted prioritization
     - [ ] On-The-Fly (runtime) Re-Prioritization
   - [ ] Dynamic route generation / Dynamic route switching
   - [ ] Context Passing pattern
   - [ ] Context extensions
   - [ ] Deactivated routes
   - [ ] Termination of continuation of checking the sub-routes by parents
 - [ ] Termination of continuation of checking the entry-routes by any
       previous routes, or sub-routes.
 - [ ] Context modification
 - [ ] Inter-Entry-Route context modification by any previous
         (sub/entry) routes
 - [ ] Entry-Route prioritization
   - [ ] Auto prioritization
   - [ ] Manual prioritization
   - [ ] Hinted prioritization
   - [ ] On-The-Fly Re-Prioritization
 - [ ] Dynamic route generation / Dynamic route switching
 - [ ] Deactivate route extension
 - [ ] Having a default constructor
 - [ ] Having a copy constructor for extensions
 - [ ] Having a move constructor for extensions
 - [ ] We need a way of saying that an extension needs another extension to work.
 - [ ] We need a way to achieve this; we need a way to specify the initial 
       context type that will be used for every single time.
